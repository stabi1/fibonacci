import pytest
import shutil
import time

from pathlib import Path

from big_int_helper import NUMER_FILES_PATH, get_all_test_numbers, get_all_dec_test_numbers


@pytest.fixture(scope="session", autouse=True)
def number_test_files(worker_id):
    base_dir: Path = NUMER_FILES_PATH
    sentinel = base_dir / ".init_done"
    is_primary = worker_id in ("master", "gw0")

    if is_primary:
        # primary: create & populate, then touch sentinel
        base_dir.mkdir(parents=True, exist_ok=True)

        for number_hex, filename in get_all_test_numbers():
            (base_dir / filename).write_text(number_hex)
        for number_hex, filename in get_all_dec_test_numbers():
            (base_dir / filename).write_text(number_hex)

        sentinel.write_text("ready")  # signal to everyone else
    else:
        # other workers: wait for sentinel
        # (timeout after, say, 30s to avoid infinite hang)
        waited = 0
        while not sentinel.exists():
            time.sleep(0.1)
            waited += 0.1
            if waited > 30:
                pytest.exit("Timed out waiting for test‐data init")

    # at this point ALL workers have the files
    yield


def pytest_sessionfinish(session, exitstatus):
    # only the xdist master/controller has no 'workerinput' attribute
    if not hasattr(session.config, "workerinput"):
        # by now *all* workers have exited
        shutil.rmtree(str(NUMER_FILES_PATH))

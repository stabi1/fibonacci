#ifndef FIBONACCI_BIGFRACSTRING_H
#define FIBONACCI_BIGFRACSTRING_H

#include "bigFrac.h"

#include <stdint.h>

bigFrac *fractionDecStringToBigFrac(const char *decStrFraction, size_t wantedBinaryDigits);

char *bigDecToDecStringFractionPart(bigFrac *x, bool exactPrecision);

#endif //FIBONACCI_BIGFRACSTRING_H

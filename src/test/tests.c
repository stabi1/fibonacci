#include "tests.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <float.h>

#include "../bigNum/bigInt/bigIntString.h"
#include "../bigNum/bigInt/bigIntDiv.h"
#include "../bigNum/bigInt/bigIntHigherFunctions.h"

#include "../bigNum/bigFrac/bigFracString.h"
#include "../bigNum/bigFrac/bigFrac.h"

char *randomHex(uint64_t n);

void testDivision();

void testTmp();

void customTest() {

    bigFrac *t = decStringToBigFrac("1538089501130828695438358760260295997583258409171372817793678011112617395943129804056658621353551964504521691472337547342520050671897761254160335892279309078582442804804903550462209250195583756381394223107483266638845726633604645582575921680810639992418330741879987048044822820892777666086210724131769915610737835384299782037288913909297084261554284152830021607524336162247543278351910887118313925543450841733312181749479293442272804889748914195275180459450533864829703202315213429156159693322388185894654982685132174201567572055035746372442741942039060059960485149178639189890783004556591699369627571102626031531346751033547236487510821902635584894013741283009410663760443340439207502003770752375245960748676895534763755692070234912347137104635763434914257425207055167983998022879645795800785855578944306858884325455359785717319124659099555362974771691031742646808036807161613733453760350587586069185073364034861302103704644404897524703876970721964930502647839243711353770897106236219832460442616158482115525878398974225082737474402904520855754495643098053144730835969939399126258517069707983979945323255021903032643583263637507016852094868817364143588307160129965933465507589849348732498985829613391903612541174481926158034713653319136758213232476781124005697930033834662698111956497252872691787070395735270844365544012315034558686683498398284478778451904670513006743568678483593432693646300504890637662423964131099097751016717984274970074663749647949.181168362269707909223138302439316426245231129529004592563155000691687396818589967117254645335071333252165770094757883981895706734867764623701447815216189092333922218128499333957269850715834306202206684974916003684536979732624269370220027679396858384812642435823717642555455708503236173678906654601057717516645699553961372418703127616547636062830048429775468899548346022638546855060457682821263214095354232210617368780895007033257347415006578548653037235084929529720735446894497934663126289700692955769880568602487512103410474818893979636728149713154835692317880769669072384259907758715741009274653409211645643460229505407249907629868415805462450244342617", true, 5);
    bigFrac *T = decStringToBigFrac("489589094045437455428537548661847553410946770469121510470451979306633907425332199631018267311231420010085345872199396403335048368010532922821744264421564367989539126061266727554992188659049066183800806424479482135198782770735455003256529801107023029072820265622803383750617873597556769529906793633910689565309478088460750377942256084526798592272024400996660234342734992402230714216565351343993249955615601190332332901273395958320532497844914211171920199166747345357895490366169471359989591186528090070593111660471384875252050870484589171700207578003630009252735195084118100713859366934310614052561386451889841333260656931065493083835133380542301777776660529450360054708067519019497503437219391795274659515403638601150623891288439333687254282687461057859632482612858399614838723220446797769397975896597668881392003888685403240099216359329338165496320010554147599532782730295814444970791632020926097436599600463202507608339673245752235061712422850246897063025598042287277293895115390300243837726766650157378502289201517967959076231927546269388771817350363619819570055856495494663235965649841831139736684172018715748208199113513948494414002048128420357334912179583109462366809676294099773721208254061411382214717092066260138970285556719338548539346430996266767390475435074571117680101987096018852147807230152286275482172361212152154959944482416381171775677445844719139904625611160621577439126195689176290709687479779821384084925739443860948085784912109375", true, 5);

    bigFrac *res1 = divideBigFrac(t, T, 53);
    printBigFracDec(res1, false);

    freeBigFrac(T);
    freeBigFrac(t);
    freeBigFrac(res1);


    /*bigFrac *tmp0 = newBigFrac(1);
    tmp0->bigIntPart->bigIntArray[0] = 0x000000000000000A;
    tmp0->fractionBlocks = 1;
    tmp0->bigIntPart->negative = true;
    printBigFracDec(tmp0, true);
    freeBigFrac(tmp0);

    printf("-----------------------------\n");

    bigFrac *tmp = newBigFrac(2);
    tmp->bigIntPart->bigIntArray[0] = 0xFFFFFFFFFFFFFFF0;
    tmp->bigIntPart->bigIntArray[1] = 0xA000000000000001;
    tmp->bigIntPart->negative = true;
    tmp->fractionBlocks = 1;
    printBigFracDec(tmp, true);
    freeBigFrac(tmp);

    printf("-----------------------------\n");

    bigFrac *tmp2 = newBigFrac(3);
    tmp2->bigIntPart->bigIntArray[0] = 0xFFFFFFFFFFFFFFFF;
    tmp2->bigIntPart->bigIntArray[1] = 0xFFFFFFFFFFFFFFFF;
    tmp2->bigIntPart->bigIntArray[2] = 0xFFFFFFFFFFFFFFFF;
    tmp2->fractionBlocks = 1;
    printBigFracDec(tmp2, true);
    freeBigFrac(tmp2);

    printf("-----------------------------\n");

    bigFrac *tmp3 = decStringToBigFrac("1.3", false, 500); // 130
    printBigIntHex(tmp3->bigIntPart);
    printBigFracHex(tmp3);
    printBigFracDec(tmp3, false);
    char *s = bigFracToHexString(tmp3);
    printf("%s\n", s);
    bigFrac *ooo = hexStringToBigFrac(s);
    printBigFracHex(ooo);
    free(s);
    freeBigFrac(ooo);
    freeBigFrac(tmp3);

    printf("A-----------------------------\n");

    bigFrac *add1 = decStringToBigFrac("1.32390843874029384792038742908470923849348579348750349857439857340958734095873405723908473298472309847", false, 1000);
    bigFrac *add2 = decStringToBigFrac("111.7", false, 100);
    bigFrac* res = addBigFrac(add1, add2);
    printBigFracDec(res, true);
    freeBigFrac(add1);
    freeBigFrac(add2);
    freeBigFrac(res);

    printf("S-----------------------------\n");

    bigFrac *sub1 = decStringToBigFrac("2343241.37", false, 1000);
    bigFrac *sub2 = decStringToBigFrac("111.7239084387402938479203874290847092384934857934875034985743985734095873409587340572390847329847230984", false, 100);
    bigFrac* res2 = subBigFrac(sub1, sub2);
    printBigFracDec(res2, true);
    freeBigFrac(sub1);
    freeBigFrac(sub2);
    freeBigFrac(res2);

    printf("D-----------------------------\n");

    bigFrac* ha1 = decStringToBigFrac("1", true, 10);
    bigFrac *ha2 = decStringToBigFrac("3", true, 10);
    bigFrac* res3 = divideBigFrac(ha1, ha2, 10);
    printBigFracDec(res3, false);
    freeBigFrac(ha1);
    freeBigFrac(ha2);
    freeBigFrac(res3);

    printf("2-----------------------------\n");
    bigFrac* ho1 = newBigFracFromBigInt(fibonacci(1000), false);
    bigFrac *ho2 = newBigFracFromBigInt(fibonacci(999), false);
    bigFrac* res4 = divideBigFrac(ho1, ho2, 2);
    printBigFracDec(res4, false);
    freeBigFrac(ho1);
    freeBigFrac(ho2);
    freeBigFrac(res4);*/

    //testTmp();
}

void testTmp() {
    char *rand = randomHex(15001 * 16);
    bigInt *a = hexStringToBigInt(rand);
    free(rand);
    char *res1 = bigIntToDecString(a, false);

    global_config.parallel = true;
    global_config.mulDepth = 1;
    global_config.convertDepth = 2;
    global_config.swap = true;
    global_config.swapThreshold = 1;
    printf("Second one\n");
    char *res2 = bigIntToDecString(a, false);

    if (strcmp(res1, res2) != 0) {
        printf("Strings not equal!!!!\n");
    } else {
        printf("Strings are equal\n");
    }
    free(res1);
    free(res2);
    freeBigInt(a);
}

void findBestValues() {
    size_t nFast;
    size_t kFast;
    double bestTime = DBL_MAX;
    uint64_t fibN = 10000000;
    printf("Beginning testing\n");
    for (size_t n = 20; n < 1000; n += 5) {
        for (size_t k = n + 2; k < n + 5000; k += 30) {
            printf("\rTesting %lu %lu", n, k);
            //naiveMulFaster = n;
            //karatsubaFaster = k;
            struct timespec start;
            if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) perror("Error measuring time!");

            bigInt *res = fibonacci(fibN);

            struct timespec end;
            if (clock_gettime(CLOCK_MONOTONIC, &end) == -1) perror("Error measuring time!");
            double time = (double) end.tv_sec - (double) start.tv_sec + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);
            if (time < bestTime) {
                bestTime = time;
                nFast = n;
                kFast = k;
                printf("\nFound new best! n: %lu, k: %lu, time: %f\n", nFast, kFast, bestTime);
            }
            freeBigInt(res);
        }
    }
}

void benchMark() {
    size_t iterations = 1; //iterations
    uint64_t n = 400000000;

    printf("Benchmark with Iterations: %ld\n", iterations);

    //code1
    struct timespec start;
    bigInt *res1;
    if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) perror("Error measuring time!");
    for (size_t i = 0; i < iterations; i++) {
        res1 = fibExpFastDoubling(n);
        freeBigInt(res1);
    }
    struct timespec end;
    if (clock_gettime(CLOCK_MONOTONIC, &end) == -1) perror("Error measuring time!");
    double time = (double) end.tv_sec - (double) start.tv_sec + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);
    printf("Time in code 1: %f\n", time);
    printf("----------------------------------------------\n");

    //code2
    struct timespec start2;
    bigInt *res2;
    if (clock_gettime(CLOCK_MONOTONIC, &start2) == -1) perror("Error measuring time!");
    for (size_t i = 0; i < iterations; i++) {
        res2 = fibWithLucas(n);
        freeBigInt(res2);
    }
    struct timespec end2;
    if (clock_gettime(CLOCK_MONOTONIC, &end2) == -1) perror("Error measuring time!");
    double time2 = (double) end2.tv_sec - (double) start2.tv_sec + 1e-9 * (double) (end2.tv_nsec - start2.tv_nsec);
    printf("Time in code 2: %f\n", time2);

    /*if (compareBigInt(res1, res2) != 0) {
        printf("Numbers not equal!\n");
    } else {
        printf("Numbers equal\n");
    }
    freeBigInt(res1);
    freeBigInt(res2);*/
}

void bruteForceDebug() {
    bool multiThread = global_config.parallel;
    global_config.parallel = false;
    if (multiThread) {
        printf("Bruteforce debug for multi-thread and single-thread\n");
    } else {
        printf("Bruteforce debug for single-thread\n");
    }
    //edge cases
    bigInt *res = newBigInt(1);
    res->bigIntArray[0] = 0;
    bigInt *res2 = fibonacci(0);
    bigInt *res3 = NULL;
    if (multiThread) {
        global_config.parallel = true;
        res3 = fibonacci(0);
        global_config.parallel = false;
    }
    if (compareBigInt(res, res2) != 0) {
        printf("Failed at 0 for single-thread\n");
        printf("%s\n", bigIntToDecString(res, true));
        printf("%s\n", bigIntToDecString(res2, true));
        if (multiThread) {
            freeBigInt(res3);
        }
        return;
    }

    if (multiThread && compareBigInt(res, res3) != 0) {
        printf("Failed at 0 for multi-thread\n");
        freeBigInt(res);
        freeBigInt(res2);
        freeBigInt(res3);
        return;
    }
    freeBigInt(res2);
    if (multiThread) {
        freeBigInt(res3);
    }
    res->bigIntArray[0] = 1;
    res2 = fibonacci(1);
    if (multiThread) {
        global_config.parallel = true;
        res3 = fibonacci(1);
        global_config.parallel = false;
    }
    if (compareBigInt(res, res2) != 0) {
        printf("Failed at 1 for single-thread\n");
        printf("%s\n", bigIntToDecString(res, true));
        printf("%s\n", bigIntToDecString(res2, true));
        if (multiThread) {
            freeBigInt(res3);
        }
        return;
    }
    if (multiThread && compareBigInt(res, res3) != 0) {
        printf("Failed at 1 for multi-thread\n");
        freeBigInt(res);
        freeBigInt(res2);
        freeBigInt(res3);
        freeBigInt(res3);
        return;
    }
    freeBigInt(res);
    freeBigInt(res2);
    if (multiThread) {
        freeBigInt(res3);
    }

    while (true) {
        bigInt *fibMinus2 = newBigInt(1);
        bigInt *fibMinus1 = newBigInt(1);
        bigInt *fib;
        fibMinus2->bigIntArray[0] = 0;
        fibMinus1->bigIntArray[0] = 1;

        size_t i = 1;
        while (i < 0xffffffffffffffff) {
            printf("\rTesting %luth fibonacci number", i + 1);
            fib = add(fibMinus1, fibMinus2);
            freeBigInt(fibMinus2);
            fibMinus2 = fibMinus1;
            fibMinus1 = fib;
            res2 = fibonacci(i + 1);
            if (compareBigInt(fibMinus1, res2) != 0) {
                printf("Failed at %lu for single-thread\n", i + 1);

                printf("%s\n", bigIntToHexString(fibMinus1));
                printf("%s\n", bigIntToHexString(res2));

                freeBigInt(fibMinus2);
                freeBigInt(fibMinus1);
                freeBigInt(res2);
                if (multiThread) {
                    freeBigInt(res3);
                }
                return;
            }
            if (multiThread) {
                global_config.parallel = true;
                res3 = fibonacci(i + 1);
                global_config.parallel = false;
                if (compareBigInt(fibMinus1, res3) != 0) {
                    printf("Failed at %lu for multi-thread\n", i + 1);
                    freeBigInt(fibMinus2);
                    freeBigInt(fibMinus1);
                    freeBigInt(res2);
                    freeBigInt(res3);
                    return;
                }
            }
            freeBigInt(res2);
            if (multiThread) {
                freeBigInt(res3);
            }
            i++;
        }
    }
}

char *randomHex(uint64_t n) {
    char hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    char *str = malloc(n + 1);
    uint64_t i = 0;
    for (; i < n; ++i) {
        str[i] = hex[random() % 16];
    }
    str[i] = '\0';
    return str;
}

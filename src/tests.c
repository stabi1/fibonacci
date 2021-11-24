#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sysinfo.h>
#include "tests.h"
#include "main.h"
#include "mulAsm.h"
#include "mul.h"
#include "BigIntAsm.h"

char *randomHex(uint64_t n);

void test() {
    /*uint64_t n = 50000000;
    bigInt* res1 = fibExpFastDoubling(n);
    bigInt* res2 = fibExpFastDoublingMultiThread(n);
    if(!compareBigInts(res1, res2)){
        printf("Fault\n");
        return;
    }
    printf("Correct\n");*/

    // 0EC30E4C53F6724857556F50AFC80013B0995173248CEF4D38BC099887FA83367
    //bigInt *test1 = hexStringToBigInt("0D5DBD26953B22FE57ECD1921FB7B6309CA7CAC791EB08301891182CD299E32B5");
    //bigInt *test2 = hexStringToBigInt("0210239CC78830891182CD299E32B5AC328DD3B5537EA2AE3C06799449D53DC0891182CD299E32B5F45328A31DEFAFAE9409D4B8C4C9224DD0549B82E758637D687F48A26EADD05AD2E4790DE213795E5665");

    // bigInt *test1 = hexStringToBigInt("0891182CD299E32B5");
    // bigInt *test2 = hexStringToBigInt("010239CC7883AC328DD3B50891182CD299E32B5537EA2AE3C06799449D53DCF45328A31D0891182CD299E32B5EFAFAE9409D4B8C4C9224DD0549B82E758637D687F48A26EADD05AD2E4790DE213795E5665");

    bigInt *test1 = hexStringToBigInt("091F95C012469CF51B3F2301676D57142AF0216B58586FFBFCE71423398033050FF6E792FE74DD0378E5AE71B340102BABF12E11A58D527F177FE2FA327E011D8");
    bigInt *test2 = hexStringToBigInt("0146686D895A6479B936BBB9FF822A9133642817AF0C16E9E7490FF0DD674D367EAC492BA2C0168F936F3F4B28C2F5C358D5E677D7E48BDE119A24D5F72B5C5392");

    bigInt *res1 = naivMul_AsmArbeit(test1, test2);
    bigInt *res2 = naivMul_AsmVergleich(test1, test2);
    printBigInt(test1);
    printBigInt(test2);
    printf("Result\n");
    if(!compareBigInts(res1, res2)){
        printf("False\n");
    }
    printBigInt(res1);
    printBigInt(res2);
    freeBigInt(test1);
    freeBigInt(test2);
    freeBigInt(res1);
    freeBigInt(res2);

    //bigInt *test1 = hexStringToBigInt(randomHex(500000));
    //bigInt *test2 = hexStringToBigInt(randomHex(500000));
    //bigInt *test1 = hexStringToBigInt("7488AF91E7EB32048768C983B67BE2A50219DFB7B73D728F01996D910ADCC91D685EA36B232CD782401996D910ADCC91D685EA36B232CD782400488AF91E7EB32048768C983B67BE2A50219DFB7B73D728F02048768");
    //bigInt *test2 = hexStringToBigInt("1996D910ADCC91D685EA36B232CD782400488AF91E7EB3204871996D910ADCC91D685EA36B232CD782400488AF91E7EB32048768C983B67BE2A50219DFB7B73D728F0728F01996D910ADCC91D685EA36B232CD78240");
    /*bigInt *test1 = hexStringToBigInt(
            "0845E50764E1CF11B9769984CD311E014B4916498E47A9A2713036AC3D9E25917B169A49AF82317D790A0C9649A55EFAE73889888F12CCF165FFE6070A30FF94F6242B0BB8A4644BA2162A88532568DE0CA5063A0F0A2FAA12CB9086D344F67A70FF5D72B94A98648D7CDAFD5B9D6F4F0029D32F6B254D52780CC7CFBD84F3791BE57FFA6257EC6F52783F8384DBE02F4371F378FB1C64DB914717A0F84955E13021A8DADB1BDF79008FF393F8E5F157A9AC82F4A9D948B427CF764E19CA2AFDED94627F867994456B2A4C176117D6A3AD32F5ED3955A4F1D244FE4919804E88583286477C4E7BA3D341CD348E64A5C6ACED940C5E121A19724BA881DE704FA6ECF2EF29BCE656A7591A77F8BE33D8E34DDFC154D0272BEDCA51333F2D4CE80AC00D7858DA2912C05646FE8DCC0DE8E5A4B939068912DEAC2D352BB6D85CA9111369068E51FFD2E1DE82E612B39E94603079AC823DAE1333E15E30CDB0038C033814E050C128ADDB1A325FB633C28A2424591ADA9F6A76FFF53220085A0D32CCD1D56510B77C47ABBD648484217E96D7D3DB1BD13D5900A62E4A59C017547CD779051DD58F27450CE11604CDEFD411FDB5D455C1FB2284BD0C1D0BC1D5821173EF84651C60A30433C1734232334FBC61D2909559A9D4FC137E521FB6173E330DDFADEEB48EBAC3C2E7E6E53932C50ED7074930F23FEA427FE6DE8E6A05389C195D9BE0809F8968E5F0EBDBE46B5339312B122A10D0E626E2DE6226FE1FC62D8B891D3866191FD5B953856D5891DCCBB9BD9654468464AC00071ECA564F8AF002B4C8796B55525525AD549B67C01550AB5F435038C35A61BF152176AD8FAAB2FF17E88387451E8B8B9F446391D7F3C8E79C3C69D322DD59AB09C5305E65DFEE2CA65C8195201A869484149A2E34F260C46CC9EBB7C5A373250175355AB35A8ECBB");
    bigInt *test2 = hexStringToBigInt(
            "0127FC1A2FE560F6AB792A645B27B79AAF5D75A2DACE2EC104DDD689DF920131C4C64A42BCC5D2FEFC83F3F5A59A674945EDE35C2A95678E3AA499AB5E7BEDA090AF2479F9644151841EE72F5AA195A4EDD83A2A7052CF67B259C61A139E5BA52DD4A0BAC2565F0EF704CA4A91C12D1577116E2FEC1623BF9EF0B33F759504A412C0228FCCB823142BFAD5909EE9D967704ED0FDCBC3398E72E9B0774136A61712997FF69424B7BCFE335851C4B63D5F69E7F9FFA844BA57A027CEA537F73588A2D7A2E54817BD8EAE6381FAB127DE73A65D1707418C183A479FB1F9B112E3583FA22D9A1623BDEE832DC134CFE8127809F5693D6FF4ACE5FDF09A3162A6B592508214DE99773BFC125CB44A5396B52EC93508467F426FAD53704ACBC79C709B885138AB3508944E188B67725C9FB9A7ACBC5BA45824C300CBEC5BBCFE170F75D3925121DAEB179D1A4A6E7899925A660D84D0C171849C1F5760883A54C546BC3B74A8CFBDF3FD8BA1497401CDD96856ED88ECC7B48A2B2812FF796AC667A51B3F52FD15C132EE82637340E816B5468E43225D7A9084427BC75EC01C246FFF0EE8B9FFA358497A5086F6171B888BF9467F73EADA640DCD24A412DCB416A1A326E549CDDC865606078CB494EFE72541E484662E809C7061AA66E2E655E383165E977AD40A6CF9158AFC4B4EAD9AB4791BD1F1E74B24C19DAAB4B1E6A57D634BB716247139ACB99B883CB69A71E4728C26BC224EF29AFE019CB222DD67406540849330BFDE3C5D35E6E744A28FC8D438E264593E5AE8460260265F72D0C7096E6F005404C4EA9593094EFC7D9C550A980A3E146F55F27DBE9B2E8A82DD09A9CF8799691DCBCBB2A3E0FC30C161FE56B25A50E048A1729FD06DB880D58F2D19988D24025AF19A5C4F11B2C92443FA5E2914DA01877D3B68F094380E4BC9F11A7DD67F");
    printBigInt(test1);
    printBigInt(test2);
    printf("\n");

    bigInt *res1 = karatsuba(test1, test2);
    bigInt *res2 = multiplyToomCook3(test1, test2);

    if (!compareBigInts(res1, res2)) {
        printf("\nFailed\n");
        printBigInt(res1);
        printBigInt(res2);
        printf("Len 1: %zu len2; %zu\n", res1->end, res2->end);
    } else {
        printf("Correct\n");
        printBigInt(res2);
    }

    freeBigInt(res1);
    freeBigInt(res2);
    freeBigInt(test1);
    freeBigInt(test2);*/

    //bigInt *test1 = hexStringToBigInt("4CC48B320965B58391BEA4169868686C0"); //soll 1996D910ADCC91D685EA36B232CD78240
    /*bigInt *test1 = hexStringToBigInt("075DA5DF7C5790D7D8112A6CD90B3D6805C5AA4B289C255F4F011FA3C2B9DECE19CB068395FAFB08A");
    printBigInt(test1);
    bigInt *res = shiftRight_Asm(test1, 4);
    printBigInt(res);
    freeBigInt(test1);
    freeBigInt(res);*/
    /*bigInt *test1 = hexStringToBigInt("8D9E53C96D6339B90D3947809B77233C48D8FBC030DD63D345BDF10A497908D024EF06F8DDB651C12F30076192916EE498F3BED1F166FE8D8EFD46D9F9EDAA150B5900C22DD2B185702195FCBA7919");
    size_t largest = 11;
    size_t k = (largest + 2) / 3;
    size_t r = largest - 2 * k;
    printf(" k:%zu r:%zu lagest:%zu testLen:%zu\n",k, r,largest, test1->end - test1->start);
    bigInt *slicesA[3];
    getToomSlice(test1, k, r, largest, slicesA);
    bigInt *a2 = slicesA[2];
    bigInt *a1 = slicesA[1];
    bigInt *a0 = slicesA[0];
    printBigInt(a2);
    printBigInt(a1);
    printBigInt(a0);
    freeBigInt(test1);
    freeBigInt(a2);
    freeBigInt(a1);
    freeBigInt(a0);*/
}

void benchMark() {
    size_t n = 1; //wiederholungen

    bigInt *test1 = hexStringToBigInt(randomHex(1000000));
    bigInt *test2 = hexStringToBigInt(randomHex(1000000));

    //code1
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (size_t i = 0; i < n; i++) {
        bigInt *res1 = naivMul_AsmVergleich(test1, test2);
        freeBigInt(res1);
    }
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);
    double time = (double) end.tv_sec - (double) start.tv_sec + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);
    printf("Time in code 1: %f\n", time);

    //code2
    struct timespec start2;
    clock_gettime(CLOCK_MONOTONIC, &start2);
    for (size_t i = 0; i < n; i++) {
        bigInt *res2 = naivMul_AsmArbeit(test1, test2);
        freeBigInt(res2);
    }
    struct timespec end2;
    clock_gettime(CLOCK_MONOTONIC, &end2);
    double time2 = (double) end2.tv_sec - (double) start2.tv_sec + 1e-9 * (double) (end2.tv_nsec - start2.tv_nsec);
    printf("Time in code 2: %f\n", time2);

    freeBigInt(test1);
    freeBigInt(test2);
}

void bruteForceDebug() {
    //edge cases
    bigInt *res = newBigInt(1);
    res->bigIntArray[0] = 0;
    bigInt *res2 = fibExpFastDoubling(0);
    bigInt *res3 = fibExpFastDoublingMultiThread(0);
    if (!compareBigInts(res, res2)) {
        printf("Failed at 0 for normal\n");
        freeBigInt(res);
        freeBigInt(res2);
        freeBigInt(res3);
        return;
    }
    if (!compareBigInts(res, res3)) {
        printf("Failed at 0 for Multithread\n");
        freeBigInt(res);
        freeBigInt(res2);
        freeBigInt(res3);
        return;
    }
    freeBigInt(res2);
    freeBigInt(res3);
    res->bigIntArray[0] = 1;
    res2 = fibExpFastDoubling(1);
    res3 = fibExpFastDoublingMultiThread(1);
    if (!compareBigInts(res, res2)) {
        printf("Failed at 1 for normal\n");
        printBigInt(res);
        printBigInt(res2);
        freeBigInt(res);
        freeBigInt(res2);
        freeBigInt(res3);
        return;
    }
    if (!compareBigInts(res, res3)) {
        printf("Failed at 1 for Multithread\n");
        freeBigInt(res);
        freeBigInt(res2);
        freeBigInt(res3);
        return;
    }
    freeBigInt(res);
    freeBigInt(res2);
    freeBigInt(res3);

    while (true) {
        bigInt *fibMinus2 = newBigInt(1);
        bigInt *fibMinus1 = newBigInt(1);
        bigInt *fib;
        fibMinus2->bigIntArray[0] = 0;
        fibMinus1->bigIntArray[0] = 1;

        size_t i = 1;
        while (i < 0xffffffffffffffff) {
            printf("\rTesting %luth fibonacci number", i);
            fib = smartAdd(fibMinus1, fibMinus2);
            freeBigInt(fibMinus2);
            fibMinus2 = fibMinus1;
            fibMinus1 = fib;
            res2 = fibExpFastDoubling(i + 1);
            if (!compareBigInts(fibMinus1, res2)) {
                printf("Failed at %lu for normal\n", i + 1);

                printBigInt(fibMinus1);
                printBigInt(res2);

                freeBigInt(fib);
                freeBigInt(fibMinus2);
                freeBigInt(fibMinus1);
                freeBigInt(res2);
                freeBigInt(res3);
                return;
            }
            res3 = fibExpFastDoublingMultiThread(i + 1);
            if (!compareBigInts(fibMinus1, res3)) {
                printf("Failed at %lu for multithread\n", i + 1);
                freeBigInt(fib);
                freeBigInt(fibMinus2);
                freeBigInt(fibMinus1);
                freeBigInt(res2);
                freeBigInt(res3);
                return;
            }
            freeBigInt(res2);
            freeBigInt(res3);
            i++;
        }
    }
}

void benchMarkAdd() {
    size_t n = 5000; //wiederholungen

    bigInt *test1 = hexStringToBigInt(randomHex(10000000));
    bigInt *test2 = hexStringToBigInt(randomHex(10000000));

    //code1
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (size_t i = 0; i < n; i++) {
        //bigInt *res1 = add_AsmAlt(test1, test2, false);
        //freeBigInt(res1);
    }
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);
    double time = (double) end.tv_sec - (double) start.tv_sec + 1e-9 * (double) (end.tv_nsec - start.tv_nsec);
    printf("Time in code 1: %f\n", time);

    //code2
    struct timespec start2;
    clock_gettime(CLOCK_MONOTONIC, &start2);
    for (size_t i = 0; i < n; i++) {
        bigInt *res2 = add_Asm(test1, test2, false);
        freeBigInt(res2);
    }
    struct timespec end2;
    clock_gettime(CLOCK_MONOTONIC, &end2);
    double time2 = (double) end2.tv_sec - (double) start2.tv_sec + 1e-9 * (double) (end2.tv_nsec - start2.tv_nsec);
    printf("Time in code 2: %f\n", time2);

    freeBigInt(test1);
    freeBigInt(test2);
}

bool compareBigInts(bigInt *x, bigInt *y) {
    if (x->start != y->start && x->end != y->end && x->negative != y->negative) {
        return false;
    }
    for (size_t i = x->start; i < x->end; i++) {
        if (x->bigIntArray[i] != y->bigIntArray[i]) {
            return false;
        }
    }
    return true;
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

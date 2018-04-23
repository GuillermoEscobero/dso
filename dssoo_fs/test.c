/*
 * OPERATING SYSTEMS DESING - 16/17
 *
 * @file 	test.c
 * @brief 	Implementation of the client test routines.
 * @date	01/03/2017
 */

#include <stdio.h>
#include <string.h>
#include "include/filesystem.h"

#include <stdlib.h>
#include <fcntl.h>

// Color definitions for asserts
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_BLUE   "\x1b[34m"

#define N_BLOCKS    25                        // Number of blocks in the device
#define DEV_SIZE    N_BLOCKS * BLOCK_SIZE    // Device size, in bytes

#define TEST_PASSED(test_name...) fprintf(stdout, "%s%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST ", test_name, ANSI_COLOR_GREEN, " SUCCESS\n\n", ANSI_COLOR_RESET)
#define TEST_FAILED(test_name...) fprintf(stdout, "%s%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST ", test_name, ANSI_COLOR_RED, " FAILED\n\n", ANSI_COLOR_RESET)
#define TEST_PRINT(test_number, requirement_tested...) fprintf(stdout, "%s%s%s%s%s", "TEST NUMBER ", test_number, ": REQUIREMENT ", requirement_tested, "\n")

int main() {

    //FUNCTIONAL REQUIREMENTS TESTS
    fprintf(stdout, "%s", "BASIC TESTING OF FUNCTIONAL REQUIREMENTS\n\n");
    
    fprintf(stdout, "%s", "This test is designed to run on disks that have a minimum size of 3000 blocks, please run \"./create_disk 3000\" before running this test\n");

    /**
     * @test 1
     * @requirement F1.1, F9, NF6
     * @description Create a file system. The file system can be created on partitions of the device smaller than
     * its maximum size. The file system will be used on disks from 50 KiB to 10 MiB.
     */
    TEST_PRINT("1", "F1.1, F9, NF6");
    if (mkFS(24 * BLOCK_SIZE) == -1 && mkFS(BLOCK_SIZE * 3000) == 0 && mkFS(5121 * BLOCK_SIZE) == -1) {
        TEST_PASSED("mkFS");
    } else {
        TEST_FAILED("mkFS");
        return -1;
    }

    /**
     * @test 2
     * @requirement F1.2
     * @description Mount a file system
     */
    TEST_PRINT("2", "F1.2");
    if (mountFS() == 0) {
        TEST_PASSED("mountFS");
    } else {
        TEST_FAILED("mountFS");
    }

    /**
     * @test 3
     * @requirement F1.4
     * @description Create a file within the file system
     */
    TEST_PRINT("3", "F1.4");
    if (createFile("test.txt") == 0 && createFile("test.txt") == -1) { // NOLINT
        TEST_PASSED("createFile");
    } else {
        TEST_FAILED("createFile");
    }

    /**
     * @test 4
     * @requirement F1.6
     * @description Open an existing file
     */
    TEST_PRINT("4", "F1.6");
    int testFileDescriptor = openFile("test.txt");
    if (testFileDescriptor == 0 && openFile("random.txt") == -1) {
        TEST_PASSED("openFile");
    } else {
        TEST_FAILED("openFile");
    }

    // Fill the buffer with text using C standard functions to avoid introducing errors
    char *write_buffer = "#INIT#Por cuanto por parte de vos, Miguel de Cervantes, nos fue fecha relación que habíades compuesto "
                         "un libro intitulado El ingenioso hidalgo de la Mancha, el cual os había costado mucho trabajo y era muy "
                         "útil y provechoso, nos pedistes y suplicastes os mandásemos dar licencia y facultad para le poder imprimir,"
                         " y previlegio por el tiempo que fuésemos servidos, o como la nuestra merced fuese; lo cual visto por los del"
                         " nuestro Consejo, por cuanto en el dicho libro se hicieron las diligencias que la premática últimamente por nos "
                         "fecha sobre la impresión de los libros dispone, fue acordado que debíamos mandar dar esta nuestra cédula para vos,"
                         " en la dicha razón; y nos tuvímoslo por bien. Por la cual, por os hacer bien y merced, os damos licencia y facultad "
                         "para que vos, o la persona que vuestro poder hubiere, y no otra alguna, podáis imprimir el dicho libro, intitulado "
                         "El ingenioso hidalgo de la Mancha, que desuso se hace mención, en todos estos nuestros reinos de Castilla, por tiempo"
                         " y espacio de diez años, que corran y se cuenten desde el dicho día de la data desta nuestra cédula; so pena que la "
                         "persona o personas que, sin tener vuestro poder, lo imprimiere o vendiere, o hiciere imprimir o vender, por el mesmo "
                         "caso pierda la impresión que hiciere, con los moldes y aparejos della; y más, incurra en pena de cincuenta mil maravedís "
                         "cada vez que lo contrario hiciere. La cual dicha pena sea la tercia parte para la persona que lo acusare, y la otra tercia "
                         "parte para nuestra Cámara, y la otra tercia parte para el juez que lo sentenciare. Con tanto que todas las veces que "
                         "hubiéredes de hacer imprimir el dicho libro, durante el tiempo de los dichos diez años, le traigáis al nuestro Consejo, "
                         "juntamente con el original que en él fue visto, que va rubricado cada plana y firmado al fin dél de Juan Gallo de Andrada, "
                         "nuestro Escribano de Cámara, de los que en él residen, para saber si la dicha impresión está conforme el original; o "
                         "traigáis fe en pública forma de cómo por corretor nombrado por nuestro mandado, se vio y corrigió la dicha impresión por el "
                         "original, y se imprimió conforme a él, y quedan impresas las erratas por él apuntadas, para cada un libro de los que así "
                         "fueren impresos, para que se tase el precio que por cada volume hubiéredes de haber. Y mandamos al impresor que así imprimiere "
                         "el dicho libro, no imprima el principio ni el primer pliego dél, ni entregue más de un solo libro con el original al autor, o"
                         " persona a cuya costa lo imprimiere, ni otro alguno, para efeto de la dicha correción y tasa, hasta que antes y primero el "
                         "dicho libro esté corregido y tasado por los del nuestro Consejo; y, estando hecho, y no de otra manera, pueda imprimir el dicho "
                         "principio y primer pliego, y sucesivamente ponga esta nuestra cédula y la aprobación, tasa y erratas, so pena de caer e incurrir "
                         "en las penas contenidas en las leyes y premáticas destos nuestros reinos. Y mandamos a los del nuestro Consejo, y a otras "
                         "cualesquier justicias dellos, guarden y cumplan est";
    /**
     * @test 5
     * @requirement F1.9, F7
     * @description Write to an opened file. A file could be modified by means of write operations.
     */
    TEST_PRINT("5", "F1.9, F7");
    if (writeFile(testFileDescriptor, write_buffer, sizeof(write_buffer) - sizeof(char) * 8) ==
        sizeof(write_buffer) - sizeof(char) * 8) {
        lseekFile(testFileDescriptor, 0, FS_SEEK_BEGIN);
        if (writeFile(testFileDescriptor, write_buffer, sizeof(char) * 8) == sizeof(char) * 8) {
            TEST_PASSED("writeFile");
        } else {
            TEST_FAILED("writeFile");
        }
    } else {
        TEST_FAILED("writeFile");
    }

    lseekFile(testFileDescriptor, 0, FS_SEEK_BEGIN);

    /**
     * @test 6
     * @requirement F1.8
     * @description Read from an opened file
     */
    TEST_PRINT("6", "F1.8");
    char read_buffer[sizeof(write_buffer)];
    if (readFile(testFileDescriptor, read_buffer, sizeof(char) * 3) == sizeof(char) * 3) {
        if (readFile(testFileDescriptor, read_buffer, sizeof(write_buffer)) == sizeof(write_buffer) - 3) {
            if (readFile(testFileDescriptor, read_buffer, sizeof(char)) == 0) {
                TEST_PASSED("readFile");
            } else {
                TEST_FAILED("readFile");
            }
        } else {
            TEST_FAILED("readFile");
        }
    } else {
        TEST_FAILED("readFile");
    }

    /**
     * @test 7
     * @requirement F1.10
     * @description Modify the position of the seek pointer
     */
    TEST_PRINT("7", "F1.10");
    char read_string[8] = {0};
    char *init_string = "#INIT#";
    if (lseekFile(testFileDescriptor, 10, FS_SEEK_BEGIN) == 0) {
        readFile(testFileDescriptor, read_string, sizeof(char) * 6);
        if (strcmp(read_string, init_string) == 0) {
            TEST_PASSED("lseekFile");
        } else {
            TEST_FAILED("lseekFile");
        }
    } else {
        TEST_FAILED("lseekFile");
    }

    /**
     * @test 8
     * @requirement F1.7
     * @description Close an opened file
     */
    TEST_PRINT("8", "F1.7");
    if (closeFile(testFileDescriptor) == 0) {
        TEST_PASSED("closeFile");
    } else {
        TEST_FAILED("closeFile");
    }

    // Testing writing on the device
    unmountFS();
    mountFS();

    /**
     * @test 9
     * @requirement F2
     * @description Every time a file is opened, its seek pointer will be reset to the beginning of the file.
     */
    TEST_PRINT("9", "F2");
    testFileDescriptor = openFile("test.txt");
    readFile(testFileDescriptor, read_buffer, sizeof(char) * 6);
    if (strcmp(read_buffer, init_string) == 0) {
        TEST_PASSED("lseek reset on openFile");
    } else {
        TEST_FAILED("lseek reset on openFile");
    }

    /**
     * @test 10
     * @requirement F1.11
     * @description Check the integrity an existing file
     */
    TEST_PRINT("10", "F1.11");
    if (checkFile("test.txt") == -2) {
        closeFile(testFileDescriptor);
        if (checkFile("test.txt") == 0) {
            bwrite("disk.dat", 5, write_buffer);
            if (checkFile("test.txt") == -1) {
                TEST_PASSED("checkFile");
            } else {
                TEST_FAILED("checkFile");
            }
        } else {
            TEST_FAILED("checkFile");
        }
    } else {
        TEST_FAILED("checkFile");
    }

    /**
     * @test 11
     * @requirement F1.5
     * @description Remove an existing file from the file system
     */
    TEST_PRINT("11", "F1.5");
    if (removeFile("test.txt") == 0 && removeFile("test.txt") == -1) { // NOLINT
        TEST_PASSED("removeFile");
    } else {
        TEST_FAILED("removeFile");
    }

    /**
     * @test 12
     * @requirement F5
     * @description File integrity must be checked, at least, on open operations.
     */
    TEST_PRINT("12", "F5");
    createFile("test.txt");
    testFileDescriptor = openFile("test.txt");
    writeFile(testFileDescriptor, write_buffer, sizeof(char) * 750);
    closeFile(testFileDescriptor);

    bwrite("disk.dat", 5, write_buffer);
    testFileDescriptor = openFile("test.txt");
    if (testFileDescriptor == -2) {
        TEST_PASSED("integrity check on openFile");
    } else {
        TEST_FAILED("integrity check on openFile");
    }
    removeFile("test.txt");

    /**
     * @test 13
     * @requirement F6
     * @description The whole contents of a file could be read by means of several read operations.
     */
    TEST_PRINT("13", "F6");
    createFile("test.txt");
    testFileDescriptor = openFile("test.txt");
    writeFile(testFileDescriptor, write_buffer, sizeof(write_buffer));
    lseekFile(testFileDescriptor, 0, FS_SEEK_BEGIN);
    int sumBytes = readFile(testFileDescriptor, read_buffer, sizeof(char) * sizeof(write_buffer) / 4); // NOLINT
    sumBytes += readFile(testFileDescriptor, read_buffer, sizeof(char) * sizeof(write_buffer) / 4); // NOLINT
    sumBytes += readFile(testFileDescriptor, read_buffer, sizeof(char) * sizeof(write_buffer) / 4); // NOLINT
    sumBytes += readFile(testFileDescriptor, read_buffer, sizeof(char) * sizeof(write_buffer) / 4); // NOLINT
    if (sumBytes == sizeof(write_buffer)) {
        TEST_PASSED("read in several readFiles");
    } else {
        TEST_FAILED("read in several readFiles");
    }

    // Read metadata for test 15
    char *metadata_buffer1 = malloc(BLOCK_SIZE);
    bread("disk.dat", 1, metadata_buffer1);

    /**
     * @test 14
     * @requirement F8
     * @description As part of a write operation, file capacity may be extended by means of additional data blocks.
     */
    TEST_PRINT("14", "F8");
    writeFile(testFileDescriptor, write_buffer, sizeof(write_buffer));
    lseekFile(testFileDescriptor, 0, FS_SEEK_BEGIN);
    if (readFile(testFileDescriptor, read_buffer, sizeof(write_buffer) * 2) == sizeof(write_buffer) * 2) {
        TEST_PASSED("extend file capacity with writeFile");
    } else {
        TEST_FAILED("extend file capacity with writeFile");
    }
    closeFile(testFileDescriptor);
    removeFile("test.txt");

    /**
     * @test 15
     * @requirement F1.3
     * @description Unmount a file system
     */
    TEST_PRINT("15", "F1.3");
    if (unmountFS() == 0) {
        TEST_PASSED("unmountFS");
    } else {
        TEST_FAILED("unmountFS");
    }

    //NON-FUNCTIONAL REQUIREMENTS TESTS
    fprintf(stdout, "%s", "BASIC TESTING OF NON-FUNCTIONAL REQUIREMENTS\n\n");

    mountFS();

    /**
     * @test 16
     * @requirement NF1
     * @description The maximum number of files in the file system will never be higher than 40.
     */
    TEST_PRINT("16", "NF1");
    char buffer[12] = {0};
    int iteration = 1;
    sprintf(buffer, "file%d.txt", iteration);

    while (createFile(buffer) == 0) {
        iteration++;
        sprintf(buffer, "file%d.txt", iteration);
    }
    if (iteration == 41) {
        TEST_PASSED("max number of files");
        for (iteration = 40; iteration > 0; --iteration) {
            sprintf(buffer, "file%d.txt", iteration);
            removeFile(buffer);
        }
    } else {
        TEST_FAILED("max number of files");
    }

    /**
     * @test 17
     * @requirement NF2
     * @description The maximum length of the file name will be 32 characters.
     */
    TEST_PRINT("17", "NF2");
    if (createFile("thisFilenameIsExactly32Chars.txt") == 0 && createFile("thisFilenameIsMoreThan32Chars.txt") == -2) {
        TEST_PASSED("file name length");
    } else {
        TEST_FAILED("file name length");
    }

    /**
     * @test 18
     * @requirement NF3
     * @description The maximum size of the file will be 1 MiB.
     */
    TEST_PRINT("18", "NF3");
    testFileDescriptor = openFile("thisFilenameIsExactly32Chars.txt");
    char *superSizedArray = calloc(MAX_FILE_SIZE * 2, sizeof(char));
    if (writeFile(testFileDescriptor, superSizedArray, sizeof(char) * MAX_FILE_SIZE * 2) == MAX_FILE_SIZE) {
        TEST_PASSED("1 MiB max file size");
    } else {
        TEST_FAILED("1 MiB max file size");
    }

    return 0;
}

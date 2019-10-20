/*
    ATTENTION: This code designed for Linux, may not works correctly in Windows.
*/

/*
    ATTENTION: The input file and the speller files must encoded as UTF-8.
*/

/*
    Programming Languages - Autumn 2017 - Assignment 3

    A program that fixes the wrong words within the given file.

    @author
    Name: Osman Araz
    Student NO: 16011020
    Date: 10.12.2017
    E-Mail: arazosman@outlook.com
    Compiler Used: GCC 7.2.0
    Computer Hardware: 64 bit Quad Core CPU, 8 GB RAM
    IDE: None (Visual Studio Code used as a text editor.)
    Operating System: Manjaro KDE Edition 17.0.6
*/

#include <stdio.h>
#include <stdlib.h>   // used for malloc(), realloc() and free() functions
#include <stdbool.h>  // used for true and false boolean values
#include <limits.h>   // used for INT_MAX value
#include <locale.h>   // used for setlocale() function
#include <wchar.h>    // used for working correctly on special Turkish characters
#include <wctype.h>   // used for towupper() and towlower() functions

// ANSI color defining
#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_RESET   "\x1b[0m"

/*
    ABOUT WIDE CHARS:

    Wide chars used on whole the program. They are the only way I found to work correctly 
    on non-ASCII Turkish characters in Linux. Some of the syntaxes, functions and their 
    explanations about the wide chars:

    wchar_t : represents a wide char variable
    wchar_t * : represents an array of wide chars (string of wide chars)
    wcslen() : a function to compute the length of a string of wide chars
    wcscpy() : a function to assign a string of wide chars to another
    wcstombs() : a function to convert a wide char string to a normal char string
    towlower() : a function to make lowercase a wide char
    towupper() : a function to make uppercase a wide char
    fgetws() : a function to reading wide chars from a file
    fputws() : a function to writing wide chars to a file

    NOTE: The initializing of a wide char/string of wide chars must prefixed by 
    letter of "L". Examples:

    wchar_t ch = L'ç';
    wchar_t *str = L"abcçdef";
*/

wchar_t ***words; // the 2-D string array of spelling files
const wchar_t *trLetters = L"ABCÇDEFGHIİJKLMNOÖPRSŞTUÜVYZ"; // Turkish characters (except "Ğ")
const int trSize = 28; // the number of Turkish characters (except "Ğ")

/*
    Main function: It gets the source file location and the number of letter distance
    from the user.
*/
void printBanner();
void crashFile();
void crashMemory();
int main()
{
    void getWords();
    void searchWord(wchar_t *, int, FILE *);

    setlocale(LC_ALL, ""); // used for display correctly the special Turskish characters
    printBanner(); // printing the banner of the program
    char fileName[50]; 
    int i, j, dist;

    getWords(); // saving the words to the array from the spelling files

    // getting informations from the user:
    printf(COLOR_YELLOW "\tHello! Please write the informations: \n\n" COLOR_RESET);
    printf("\tThe target of the text file: ");
    scanf("%s", fileName);
    printf("\tTHe letter distance: ");
    scanf("%d", &dist);

    // opening input and output files
    FILE *fi, *fo;
    fi = fopen(fileName, "r");
    fo = fopen("output.txt", "w");

    if (!fi || !fo) // checking crashes
        crashFile();

    wchar_t word[100];

    while (fgetws(word, 100, fi)) // reading words from the input file
        searchWord(word, dist, fo); // searching the word on the array of the saved words

    printBanner();
    printf(COLOR_YELLOW "\tSearch completed!\n\tPlease check the output file: \"output.txt\".\n\n" COLOR_RESET);

    // deallocation the array of the saved words
    for (i = 0; i < trSize; i++)
        free(words[i]);

    free(words);

    // closing the input and output files
    fclose(fi);
    fclose(fo);

    return 0;
}

/*
    Function which saves the words from the spelling files to the array.
*/
char *getFileTarget(wchar_t);
void getWords()
{
    int i, p;
    words = (wchar_t ***)malloc(trSize*sizeof(wchar_t **));

    if (!words) // checking crashes
        crashMemory();

    wchar_t tmp[100];

    for (i = 0; i < trSize; i++) // for all 28 letters
    {
        int j = 0, k = 1000;
        words[i] = (wchar_t **)malloc(k*sizeof(wchar_t *));

        if (!words[i]) // checking crashes
            crashMemory();

        // getting the file location according to the letter and then opening the file
        char *fileTarget = getFileTarget(trLetters[i]);
        FILE *file = fopen(fileTarget, "r");

        printf("%s\n", fileTarget);

        if (!file) // checking crashes
            crashFile();

        for (p = 0; p < k; p++)
        {
            // memory allocation for strings
            words[i][p] = (wchar_t *)malloc(30*sizeof(wchar_t));

            if (!words[i][p]) // checking crashes
                crashMemory();
        }

        while (!feof(file))
        {
            while (j < k && !feof(file))
            {
                fgetws(tmp, 100, file); // getting words from the file
                wcscpy(words[i][j], tmp); // copying words to the array
                j++;
            } 

            if (!feof(file))
            {
                k *= 2; // if any words remained, then the size of the array extends
                words[i] = realloc(words[i], k*sizeof(wchar_t *));

                if (!words[i]) // checking crashes
                    crashMemory();

                for (p = k/2; p < k; p++)
                {
                    // memory allocation for strings (for extended parts)
                    words[i][p] = (wchar_t *)malloc(30*sizeof(wchar_t));

                    if (!words[i][p]) // checking crashes
                        crashMemory();
                }
            }
        }

        fclose(file); // closing the file
    }
}

/*
    Function which searches the word on the array of the saved words.
    @param word: the word which will be searched
    @param dist: the letter distance
    @fo: the output file
*/
int compareStrings(wchar_t *, wchar_t *, int);
void addSimilarWord(wchar_t **, wchar_t *, wchar_t *, int);
void searchWord(wchar_t *word, int dist, FILE *fo)
{
    // count: represents the number of similar words
    // capacity: represents the capacity of similar words
    // isFound: represents does the word found on the speller files or not
    int i, j, p, count = 0, capacity = 100;
    bool isFound = false;

    // array of the similar words
    wchar_t **similarWords = (wchar_t **)malloc(capacity*sizeof(wchar_t *));

    if (!similarWords) // checking crashes
        crashMemory();

    for (i = 0; i < capacity; i++)
    {
        // memory allocation for similar strings
        similarWords[i] = (wchar_t *)malloc(30*sizeof(wchar_t));
        
        if (!similarWords[i]) // checking crashes
            crashMemory();
    }

    // the word will be searched on all the 28 speller files until it found
    for (i = 0; i < trSize && !isFound; i++)
    {
        j = 0;

        // a while loop to search the word on the specific speller file
        // if there is a string, wcslen() will be different than 0
        while (!isFound && wcslen(words[i][j]))
        {
            // diff: represents the letter distance between the word and the saved words
            int diff = compareStrings(word, words[i][j], dist);

            if (diff == 0) // if letter distance is 0, then the word is found
            {
                // writing the word on the output file
                fputws(L"+ ", fo);
                fputws(word, fo);
                isFound = true;
            }
            // if letter distance is greater than wanted letter distance, then diff will be -1
            else if (diff != -1)
            {
                if (count >= capacity)
                {
                    // if count is come to the capacity, then capacity will be increase
                    capacity *= 2;
                    similarWords = realloc(similarWords, capacity*sizeof(wchar_t *));

                    if (!similarWords) // checking crashes
                        crashMemory();

                    for (p = capacity/2; p < capacity; p++)
                    {
                        // memory allocation for similar strings (for extended parts)
                        similarWords[p] = (wchar_t *)malloc(30*sizeof(wchar_t));

                        if (!similarWords[p]) // checking crashes
                            crashMemory();
                    }
                }

                // the found similar word will be added on the array of the similar words
                addSimilarWord(similarWords, word, words[i][j], count); 
                count++;
            }

            j++;
        }
    }

    // if the word doesn't found on the saved words, then the similar words 
    // will be written on the output file
    if (!isFound)
    {
        fputws(L"- ", fo);

        for (i = 0; i < count; i++)
        {
            fputws(similarWords[i], fo);
            fputws(L" ", fo);
        }
    }

    fputws(L"\n", fo);

    // deallocation the array of the similar words
    for (i = 0; i < capacity; i++)
        free(similarWords[i]);

    free(similarWords);
}

/*
    Function which adds the similar words to the array of the similar words 
    according to their letter distances between the searched word.
    @param **similarWords: the array of the similar words
    @param *word: the word which searched on the array of the saved words
    @param *fileWord: the word which will be added to the array of the similar words 
    @param count: the count of the array of the similar words
*/
void addSimilarWord(wchar_t **similarWords, wchar_t *word, wchar_t *fileWord, int count)
{
    int i, j = 0;

    // the similar word will be insert to the array of the similar words 
    // according to its letter distance between the searched word
    while (j < count && compareStrings(fileWord, word, INT_MAX) > compareStrings(similarWords[j], word, INT_MAX))
        j++;

    for (i = count; i > j; i--)
        wcscpy(similarWords[i], similarWords[i-1]); // similar words are shifting to the right

    wcscpy(similarWords[j], fileWord);
}

/*
    Function which compares the letter distances between two strings.
    @param *s1: a string
    @param *s2: another string
    @param dist: the maximum letter distance which permitted
    @return diff: the letter distance between given strings, if two strings have different lentghs 
    or letter distance is bigger than the maximum letter distance, then it returns -1
*/
int compareStrings(wchar_t *s1, wchar_t *s2, int dist)
{
    // ignoring the new line characters on the end of the strings
    if (s1[wcslen(s1)-1] == '\n' || s1[wcslen(s1)-1] == ' ')
        s1[wcslen(s1)-1] = '\0';

    if (s2[wcslen(s2)-1] == '\n' || s2[wcslen(s2)-1] == ' ')
        s2[wcslen(s2)-1] = '\0';

    // computing the length of the strings
    int s1_length = wcslen(s1), s2_length = wcslen(s2);

    // if the length of the strings are unequal, then it returns -1
    if (s1_length != s2_length)
        return -1;

    int diff = 0, j = 0;

    // computing the letter distance
    while (j < s1_length && diff <= dist)
    {
        if (towlower(s1[j]) != towlower(s2[j]))
            diff++;

        j++;
    }

    // if letter distance is excesses the maximum letter distance, then it returns -1
    if (diff > dist)
        return -1;

    return diff;
}

/*
    Function which generates the file target.
    @param ch: the fifth char of the target of "words/?.txt"
    @return fileTarget: the generated file target
*/
char *getFileTarget(wchar_t ch)
{
    char *fileTarget = (char *)malloc(20*sizeof(char));
    wchar_t targetWchar[] = L"words/?.TXT";
    targetWchar[6] = towupper(ch);
    wcstombs(fileTarget, targetWchar, 20); // wchar_t * > char *

    return fileTarget;
}

/*
    Function which displays the error message about the file opening.
*/
void crashFile()
{
    printBanner();
    printf(COLOR_RED "\tFile could not be opened.\n\n" COLOR_RESET);
    exit(1);
}

/*
    Function which displays the error message about the memory allocation.
*/
void crashMemory()
{
    printBanner();
    printf(COLOR_RED "\tNot enough space.\n\n" COLOR_RESET);
    exit(1);
}

/*
    Function which prints the banner of the program.
*/
void printBanner()
{
    system("clear"); // cleaning the screen
    printf(COLOR_CYAN "\n\t#######################################\n");
    printf("\t####" COLOR_MAGENTA "          WORD FIXING          " COLOR_CYAN "####\n");
    printf("\t#######################################\n\n\n" COLOR_RESET);
}

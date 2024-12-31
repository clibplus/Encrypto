#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <str.h>
#include <arr.h>
#include <OS/file.h>

typedef struct Encrypto {
    unsigned char   **ALPHABET;
    long            idx;
    int             DigitLength;
} Encrypto;

unsigned char *Convert(char *data) {
    if(!data)
        return NULL;

    if(strlen(data) < 1)
        return NULL;

    unsigned char *new_data = (unsigned char *)malloc(strlen(data) + 1);
    memset(new_data, '\0', strlen(data) + 1);

    for(int i = 0; i < strlen(data); i++)
        new_data[i] = data[i];

    return new_data;
}

void CreateAlphabet(Encrypto *e) {
    for(int i = 0; i < 79; i++) {
        unsigned char *current = (unsigned char *)malloc(4);
        memset(current, '\0', 4);

        for(int c = 0; c < e->DigitLength; c++) {
            int ch = (rand() % 10);
            current[c] = '0' + ch;
        }

        e->ALPHABET[e->idx] = current;
        e->idx++;
        e->ALPHABET = (unsigned char **)realloc(e->ALPHABET, sizeof(unsigned char *) * (e->idx + 1));
    }
}

unsigned char *CreateHash(Encrypto *e, char *query) {
    if(!e || !query)
        return NULL;

    unsigned char *hash = (unsigned char *)malloc(strlen((const char *)query) * (e->DigitLength + 1 + 1));
    memset(hash, '\0', strlen((const char *)query) * (e->DigitLength + 1 + 1));
    int idx = 0;

    int count = strlen(query);
    for(int i = 0; i < count; i++) {
        for(int ch = 0; ch < 79; ch++) {
            if(query[i] != 'a' + ch) {
                continue;
            }
            
            strncat(hash, e->ALPHABET[ch], strlen(e->ALPHABET[ch]));
            if(i != count - 1)
                strcat(hash, "-");
        }
    }

    return hash;
}

void GetAlphabet(Encrypto *e, char *file) {
    if(!e || !file)
        return;

    
    File alpha = Openfile(file, FILE_READ);
    alpha.Read(&alpha);

    String symbol_cfg = NewString(alpha.data);
    char **SYMBOLS = symbol_cfg.Split(&symbol_cfg, "-");

    e->ALPHABET = (unsigned char **)malloc(sizeof(unsigned char *) * 1);
    memset(e->ALPHABET, '\0', sizeof(unsigned char *) * 1);

    while(SYMBOLS[e->idx] != NULL) {
        e->ALPHABET[e->idx] = (unsigned char *)malloc(strlen(SYMBOLS[e->idx]) + 1);
        if(!e->ALPHABET[e->idx])
            break;

        strcpy((char *)e->ALPHABET[e->idx], SYMBOLS[e->idx]);
        e->idx++;
        e->ALPHABET = (unsigned char **)realloc(e->ALPHABET, sizeof(unsigned char *) * (e->idx + 1));
        
        if(!e->ALPHABET)
            break;
    }

    symbol_cfg.Destruct(&symbol_cfg);
    alpha.Destruct(&alpha);
}

char *Encrypt(Encrypto *e, char *hash) {
    String n = NewString(hash);
    String encryption = NewString(NULL);

    Array arr = NewArray(NULL);
    arr.Merge(&arr, (void **)n.Split(&n, "-"));

    for(int i = 0; i < arr.idx; i++) {
        int ch = atoi(arr.arr[i]);
        encryption.AppendNum(&encryption, (int)ch / 2);
        if(i != arr.idx - 1)
            encryption.AppendString(&encryption, "-");
    }

    char *encrypted = strdup(encryption.data);
    encryption.Destruct(&encryption);

    return encrypted;
}

Encrypto InitEncrypto(char *alphabet, int digit_length) {
    Encrypto e = (Encrypto){ 
        .ALPHABET = (!alphabet ? (unsigned char **)malloc(sizeof(unsigned char *) * 1) : NULL),
        .idx = (!alphabet ? 0 : strlen(alphabet)),
        .DigitLength = (!digit_length ? 4 : digit_length)
    };

    if(alphabet) {
        GetAlphabet(&e, alphabet);
        return e;
    }

    CreateAlphabet(&e);
    return e;
}

int main(int argc, char **argv) {
    // if(argc < 2) {
    //     printf("[ x ] Error, Invalid arguments provided....\r\nUsage: %s <key_per_char>\n", argv[0]);
    // }
    srand(time(NULL));

    Encrypto e = InitEncrypto(NULL, 4);

    printf("\x1b[32mGenerated Table:\x1b[0m \n");
    for(int i = 0; i < 79; i++) {
        printf("%c:%s, ", '0' + i, (char *)e.ALPHABET[i]);
    }
    printf("\n");

    unsigned char *hashed = CreateHash(&e, "nigger");
    printf("\x1b[32mHashed Key:\x1b[0m %s\n", (char *)hashed);

    char *Encrypted = Encrypt(&e, (char *)hashed);
    printf("\x1b[32mEncryption Key:\x1b[0m %s\n", Encrypted);

    free(hashed);
    free(Encrypted);

    for(int i = 0; i < 79; i++)
        free(e.ALPHABET[i]);

    free(e.ALPHABET);
    return 0;
}
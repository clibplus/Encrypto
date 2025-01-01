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

int IsSymbolValid(Encrypto *e, char *sym) {
    for(int i = 0; i < e->idx; i++) {
        if(!strcmp((const char *)e->ALPHABET[i], sym))
            return 1;
    }

    return 0;
}

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
    for(int i = 'A'; i < 'Z' + 1; i++) {
        unsigned char *current = (unsigned char *)malloc(e->DigitLength + 1);
        memset(current, '\0', e->DigitLength + 1);

        while(IsSymbolValid(e, current) != 0) {
            for(int c = 0; c < e->DigitLength; c++) {
                int ch = ((rand() % 9) + 1);
                current[c] = '0' + ch;
            }
        }
        current[e->DigitLength - 1] = '\0';

        printf("%s\n", current);
        e->ALPHABET[e->idx] = current;
        e->idx++;
        e->ALPHABET = (unsigned char **)realloc(e->ALPHABET, sizeof(unsigned char *) * (e->idx + 1));
        e->ALPHABET[e->idx] = NULL;
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
        for(int ch = 0; ch < e->idx; ch++) {
            if(query[i] != 'a' + ch) {
                continue;
            }
            
            strncat(hash, e->ALPHABET[ch], strlen(e->ALPHABET[ch]) - 1);
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
    char **SYMBOLS = symbol_cfg.Split(&symbol_cfg, ";");

    e->ALPHABET = (unsigned char **)malloc(sizeof(unsigned char *) * 1);
    memset(e->ALPHABET, '\0', sizeof(unsigned char *) * 1);

    while(SYMBOLS[e->idx] != NULL) {
        e->ALPHABET[e->idx] = (unsigned char *)malloc(strlen(SYMBOLS[e->idx]) + 1);
        memset(e->ALPHABET[e->idx], '\0', strlen(SYMBOLS[e->idx]) + 1);
        if(!e->ALPHABET[e->idx])
            break;

        strcpy((char *)e->ALPHABET[e->idx], SYMBOLS[e->idx]);
        e->idx++;
        e->ALPHABET = (unsigned char **)realloc(e->ALPHABET, sizeof(unsigned char *) * (e->idx + 1));
        e->ALPHABET[e->idx] = NULL;
        
        if(!e->ALPHABET)
            break;
    }

    symbol_cfg.Destruct(&symbol_cfg);
    alpha.Destruct(&alpha);
}

int SaveAlphabet(Encrypto *e) {
    if(!e)
        return 0;

    String save = NewString(NULL);

    for(int i = 0; i < e->idx; i++) {
        save.AppendString(&save, (const char *)e->ALPHABET[i]);
        save.AppendString(&save, ";");
    }
    save.data[save.idx] = '\0';

    File alpha = Openfile(".enc", FILE_WRITE);
    if(!alpha.fd)
        return 0;

    alpha.Write(&alpha, save.data);
    alpha.Destruct(&alpha);
    save.Destruct(&save);

    return 1;
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
    encryption.data[encryption.idx] = '\0';

    char *encrypted = strdup(encryption.data);
    encryption.Destruct(&encryption);

    return encrypted;
}

Encrypto InitEncrypto(int from_file, int digit_length) {
    Encrypto e = (Encrypto){ 
        .ALPHABET       = (!from_file ? (unsigned char **)malloc(sizeof(unsigned char *) * 1) : NULL),
        .idx            = 0,
        .DigitLength    = (!digit_length ? 4 : digit_length)
    };

    if(from_file) {
        GetAlphabet(&e, ".enc");
        return e;
    }

    CreateAlphabet(&e);
    if(!SaveAlphabet(&e))
        printf("[ x ] Error, Unable to save db .... !\n");
    
    return e;
}

void DestructEncrypto(Encrypto *e) {
    if(e->ALPHABET) {
        for(int i = 0; i < e->idx; i++) {
            free(e->ALPHABET[i]);
            e->ALPHABET = NULL;
        }

        free(e->ALPHABET);
    }
}

int main(int argc, char **argv) {
    if(argc < 4) {
        printf("[ x ] Error, Invalid arguments provided....\r\nUsage: %s <key_per_char>\n", argv[0]);
    }
    srand(time(NULL));

    Encrypto e = InitEncrypto(atoi(argv[1]), atoi(argv[2]));
    unsigned char *hashed = CreateHash(&e, argv[3]);

    printf("\x1b[32mHashed Key:\x1b[0m %s\n", (char *)hashed);

    char *Encrypted = Encrypt(&e, (char *)hashed);
    printf("\x1b[32mEncryption Key:\x1b[0m %s\n", Encrypted);


    for(int i = 0; i < e.idx; i++) {
        if(!e.ALPHABET[i])
            break;
        
        free(e.ALPHABET[i]);
    }

    free(hashed);
    free(Encrypted);
    free(e.ALPHABET);
    return 0;
}
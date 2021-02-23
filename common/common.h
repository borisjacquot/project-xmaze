#define MAXNAME 9
#pragma pack(1)

typedef struct {
    uint16_t port;
    char name[MAXNAME];
} balise_t;

#pragma pack()

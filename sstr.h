struct sstr {
    int index; // current index in the char array
    int M; // number of segments
    int L; // length of each segment
    int maxLentgh; // max length of the string
    int checkedsegments;
    int currentsegments;
    int done; // completion flag
    char c0;
    char c1;
    char c2;
    char *charArray; // char array
    
};

struct sstr * sstr_create(struct sstr *self,int M, int L, char c0, char c1, char c2);
void sstr_count(struct sstr *self, int *returnArry);
void sstr_append(struct sstr *self, char myChar);


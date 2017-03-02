struct scounter {
    int counter;
};

void scounter_create(struct scounter *self);
int scounter_get(struct scounter *self);
void scounter_increment(struct scounter *self);
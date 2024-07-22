#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>

#define set_bit(val, pos) val |= (1<<pos)
#define get_bit(val, pos) ((val>>pos)&0x1)
#define clr_bit(val, pos) val &= ~(1<<pos)

#define S_TIMES_N 10000
volatile int finish=0;
volatile uint8_t bitmap=0;

char buffer[128];

void* task(void* args)
{
    int pos = *(int*)args;
    char filename[128];

    snprintf(filename, 128, "out_%d.dat", pos);
    FILE *fp = fopen(filename, "wb");
        
    while(!finish)
    {
        while((get_bit(bitmap, pos)) == 0){
            continue;
        }

        fwrite(buffer, 1, 128, fp);
        
        clr_bit(bitmap, pos);
    }

    fclose(fp);

    printf("task %d finish\n", pos);

    return NULL;
}


int main(int argc, char *argv[])
{
    pthread_t th[4];
    int pos[4];

    bitmap = 0;
    
    for(int i=0; i<1; ++i){
        pos[i] = 3;
        pthread_create(&th[i], NULL, task, &pos[i]);
    }

    FILE *fp = fopen("src.dat", "rb");

    while(!feof(fp))
    {
        while(bitmap){
            continue;
        }
        
        fread(buffer, 1, 128, fp);
        
        // set_bit(bitmap, 0);
        // // set_bit(bitmap, 1);
        // // set_bit(bitmap, 2);
        set_bit(bitmap, 3);
    }
    finish=1;

    for(int i=0; i<1; ++i){
        pthread_join(th[i], NULL);
    }

    fclose(fp);

    printf("finish main\n");


    return 0;
}
#include <stdio.h>
#include <stdlib.h>

typedef struct slist_s
{
    struct slist_s *next;
}slist_t;

struct list
{
    struct list *next;
    int age;
    int sex;
    float height;
    char name[32];
};
int a = sizeof(struct list);

#define slist_new_node(_type) (slist_t*)malloc(sizeof(slist_t)+sizeof(_type))

#define slist_get_elem(_node, _type) ( (_type*)(((char*)_node)+sizeof(slist_t)) )

#define slist_insert_front(_pos, _node) ({ _node->next = _pos; _pos = _node; })

#define slist_insert_back(_pos, _node) ({ _node->next = _pos->next; _pos->next = _node; })

#define slist_erase_node(_prev) ({ slist_t* tmp = _prev->next; _prev->next = tmp->next; tmp; })

#define slist_erase_reset(_prev, _node) ({ _node = _prev; })

#define slist_foreach(head, node) \
    for(node=head; node; node=node->next)

#define slist_foreach_safe(head, prev, node) \
    for(prev=head, node=prev->next; node; prev=node, node=node->next)

/**********************************/
#include <string.h>

typedef struct info_t
{
    int age;
    char name[32];
}info_t;

int main(void)
{
    slist_t *node, *tmp, *prev, *head = slist_new_node(info_t);
    slist_get_elem(head, info_t)->age = -1;
    snprintf(slist_get_elem(head, info_t)->name, 32, "tom");

    for(int i=0; i<10; ++i){
        node = slist_new_node(info_t);
        
        slist_get_elem(node, info_t)->age = 10+i;
        snprintf(slist_get_elem(node, info_t)->name, 32, "tom%d", i);

        slist_insert_back(head, node);
    }

    printf("----------split-------------\n");
    slist_foreach(head, node){
        printf("name=%s age=%d\n", slist_get_elem(node, info_t)->name, slist_get_elem(node, info_t)->age);
    }

    printf("----------split-------------\n");
    slist_foreach_safe(head, prev, node){
        if(slist_get_elem(node, info_t)->age<15){
            tmp = slist_erase_node(prev);
            
            free(tmp);
            tmp = NULL;

            slist_erase_reset(prev, node);
        }
    }

    printf("----------split-------------\n");
    slist_foreach(head, node){
        printf("name=%s age=%d\n", slist_get_elem(node, info_t)->name, slist_get_elem(node, info_t)->age);
    }

    return 0;
}

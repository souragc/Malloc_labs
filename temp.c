/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "3agl3",
    /* First member's full name */
    "sourag",
    /* First member's email address */
    "souragc2001@gmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8 
/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(unsigned int)*2))
///#define SIZE_T_SIZE 8
char * prevcall;
char * head;
char * firstchunk;
int hook=1;
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    hook=1;
    prevcall=NULL;
    head=NULL;
    firstchunk=NULL;
    return 0;
}

#define SETPREV(ptr,val) (*((unsigned int *)ptr)=(val&~0x7))
#define SETSIZE(ptr,val) (*(((unsigned int *)ptr)+1)=val)
#define GETPREVSIZE(ptr) (*((unsigned int *)ptr)&~0x7)
#define GETSIZE(ptr) (*(((unsigned int *)ptr)+1))
#define GETREALSIZE(ptr) (*(((unsigned int *)ptr)+1)&~0x7)
#define GETNEXTADDR(ptr) (*(((unsigned int **)ptr)+2))
#define GETPREVADDR(ptr) (*(((unsigned int **)ptr)+3))
#define SETNEXTADDR(ptr,val) (*(((unsigned int **)ptr)+2)=(unsigned int *)val)
#define SETPREVADDR(ptr,val) (*(((unsigned int **)ptr)+3)=(unsigned int *)val)


/*  #define SETPREVBITZERO(ptr) (SETSIZE(ptr,(GETSIZE(ptr)&~0x1)))
#define SETNEXTBITZERO(ptr) (SETSIZE(ptr,(GETSIZE(ptr)&~0x2)))

#define SETPREVBITONE(ptr) (SETSIZE(ptr,(GETSIZE(ptr)|0x1)))
#define SETNEXTBITONE(ptr) (SETSIZE(ptr,(GETSIZE(ptr)|0x2)))*/

void removefromlist(char * ptr){
    if(ptr==head){
        head=(char *)GETNEXTADDR(ptr);
        if(GETNEXTADDR(ptr)!=NULL)
            SETPREVADDR(GETNEXTADDR(ptr),NULL);
        SETNEXTADDR(ptr,NULL);
        SETPREVADDR(ptr,NULL);
    }
    else if(GETNEXTADDR(ptr)==NULL){
        SETNEXTADDR(GETPREVADDR(ptr),NULL);
        SETPREVADDR(ptr,NULL);
    }
    else{
        SETPREVADDR(GETNEXTADDR(ptr),GETPREVADDR(ptr));
        SETNEXTADDR(GETPREVADDR(ptr),GETNEXTADDR(ptr));
        SETPREVADDR(ptr,NULL);
        SETNEXTADDR(ptr,NULL);

    }
}

void split(char * ptr,int size){
    int newsize=ALIGN(size+SIZE_T_SIZE);
    char * newptr=ptr+newsize;
    char * prevptr=ptr-GETPREVSIZE(ptr);
    char * nextptr=ptr+GETREALSIZE(ptr);
    SETSIZE(newptr,((GETREALSIZE(ptr)-newsize)|0x3));
    SETSIZE(prevptr,(GETSIZE(prevptr)|0x2));
    SETPREV(newptr,newsize);
    SETSIZE(ptr,(newsize|0x5));
    SETPREV(nextptr,GETREALSIZE(newptr));
    //SETNEXTADDR(ptr,NULL);
    //SETPREVADDR(ptr,NULL);
    if(GETREALSIZE(newptr)>16){
        if(head==NULL){
            head=newptr;
            SETNEXTADDR(head,NULL);
            SETPREVADDR(head,NULL);
        }
        else{
            SETPREVADDR(head,newptr);
            SETNEXTADDR(newptr,head);
            SETPREVADDR(newptr,NULL);
            head=newptr;
        }
    }
}
void setprevbit(char * ptr){
    if(prevcall==NULL)
        SETSIZE(ptr,(GETSIZE(ptr)|0x1));
    else if(!(GETSIZE(prevcall)&(1<<2)))
        SETSIZE(ptr,(GETSIZE(ptr)&~0x1));
    else
        SETSIZE(ptr,(GETSIZE(ptr)|0x1));
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    int newsize = ALIGN(size + SIZE_T_SIZE);
    if(hook==1){
        hook=0;
        void *p = mem_sbrk(newsize);
        SETPREV(p,0);
        SETSIZE(p,(newsize|0x6));
        setprevbit(p);
        prevcall=(char *)p;
        firstchunk=(void *)p;
        return (void *)((char *)p + SIZE_T_SIZE);
    }   
    else{
        if(head!=(char *)NULL){
            char *temp=(char *)head;
            char* temptemp=head;
            while(GETNEXTADDR(temp)!=(unsigned int *)NULL){
                if(GETREALSIZE(temp)==newsize)
                    break;
                temp=(char *)GETNEXTADDR(temp);
            }
            if(GETREALSIZE(temp)==newsize){
                SETSIZE(temp,(GETSIZE(temp)|0x4));
                removefromlist(temp);
                if(temp!=firstchunk){
                    char * prevptr=temp-GETPREVSIZE(temp);
                    SETSIZE(prevptr,(GETSIZE(prevptr)|0x2));
                }
                if(temp!=prevcall){
                    char * nextptr=temp+GETREALSIZE(temp);
                    SETSIZE(nextptr,(GETSIZE(nextptr)|0x1));
                }
                return (void *)((char *)temp+SIZE_T_SIZE);
            }
            temp=temptemp;
            while(GETNEXTADDR(temp)!=(unsigned int*)NULL){
                if(GETREALSIZE(temp)>newsize)
                    break;
                temp=(char *)GETNEXTADDR(temp);
            }
            if(GETREALSIZE(temp)>newsize){
                removefromlist(temp);
                split(temp,newsize);
                return (void *)((char *)temp+SIZE_T_SIZE);
            }
            else{
                void *p=mem_sbrk(newsize);
                SETPREV(p,GETREALSIZE(prevcall));
                SETSIZE(p,(newsize|0x6));
                setprevbit(p);
                prevcall=(void *)p;
                return (void *)((char *)p+SIZE_T_SIZE);
            }
        }
        else{
            void *p=mem_sbrk(newsize);
            SETPREV(p,GETREALSIZE(prevcall));
            SETSIZE(p,(newsize|0x6));
            setprevbit(p);
            prevcall=(char *)p;
            return (void *)((char *)p +SIZE_T_SIZE);
        }

    }
}
/*void
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr){
    char * newptr=(char *)(ptr-SIZE_T_SIZE);
    SETSIZE(newptr,(GETSIZE(newptr)&~0x4));
    int size=GETSIZE(newptr);
    memset(ptr,'\x00',(GETREALSIZE(newptr)-SIZE_T_SIZE));
    char * nextptr=newptr+GETREALSIZE(newptr);
    char * prevptr=newptr-GETPREVSIZE(newptr);
    if(!(size&(1<<0)) && !(size&(1<<1))){
        if(!(size&(1<<0)) && newptr!=firstchunk){
            if(newptr!=prevcall){
                SETSIZE(nextptr,(GETSIZE(nextptr))&~0x1);
            }
            SETSIZE(prevptr,(GETSIZE(prevptr)+GETREALSIZE(newptr)));
            SETSIZE(newptr,(unsigned int)NULL);
            SETPREV(newptr,(unsigned int)NULL);
        }
        if(!(size&(1<<1)) && newptr!=prevcall){
            char * nextnextptr=nextptr+GETREALSIZE(nextptr);
            removefromlist(nextptr);
            SETSIZE(prevptr,(GETSIZE(prevptr)+GETREALSIZE(nextptr)));
            SETPREV(nextnextptr,GETREALSIZE(prevptr));
            SETPREV(nextptr,(unsigned int)NULL);
            SETSIZE(nextptr,(unsigned int)NULL);
            SETPREVADDR(nextptr,NULL);
            SETNEXTADDR(nextptr,NULL);
        }
    }
    else if(!(size&(1<<0)) && newptr!=firstchunk){
        char * prevptr=newptr-GETPREVSIZE(newptr);
        char * nextptr=newptr+GETREALSIZE(newptr);
        SETSIZE(prevptr,(GETSIZE(prevptr)+GETREALSIZE(newptr)));
        SETPREV(nextptr,GETREALSIZE(prevptr));
        SETSIZE(nextptr,(GETSIZE(nextptr)&~0x1));
        SETPREV(newptr,(unsigned int)NULL);
        SETSIZE(newptr,(unsigned int)NULL);
    }
    else if(!(size&(1<<1)) && newptr!=prevcall){
        char * prevptr=newptr-GETPREVSIZE(newptr);
        char * nextptr=newptr+GETREALSIZE(newptr);
        removefromlist(nextptr);
        SETSIZE(newptr,((GETREALSIZE(newptr)+GETREALSIZE(nextptr))|0x3));
        char * nextnextptr=nextptr+GETREALSIZE(nextptr);
        SETPREV(nextnextptr,GETREALSIZE(newptr));
        SETSIZE(prevptr,(GETSIZE(prevptr)&~0x2));
        SETSIZE(nextptr,(unsigned int)NULL);
        SETPREV(nextptr,(unsigned int)NULL);
        SETNEXTADDR(nextptr,NULL);
        SETPREVADDR(nextptr,NULL);
        if(newptr==firstchunk)
            SETSIZE(newptr,(GETSIZE(newptr)|0x2));
        if(head==NULL){
            head=newptr;
            SETNEXTADDR(head,NULL);
            SETPREVADDR(head,NULL);
        }
        else{
            SETPREVADDR(head,newptr);
            SETNEXTADDR(newptr,head);
            head=newptr;
        }
    }
    else{
        if(newptr!=firstchunk){
            SETSIZE(prevptr,(GETSIZE(prevptr)&~0x2));
        }
        if(newptr!=prevcall){
            SETSIZE(nextptr,(GETSIZE(nextptr)&~0x1));}
        if(head==NULL){
            head=newptr;
            SETNEXTADDR(head,NULL);
            SETPREVADDR(head,NULL);
        }
        else{
            SETPREVADDR(head,newptr);
            SETNEXTADDR(newptr,head);
            head=newptr;
            SETPREVADDR(newptr,NULL);
        }
    }
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    char * newptr=(char *)ptr-SIZE_T_SIZE;
    if(ptr==NULL)
        mm_malloc(size);
    else if(size==0){
        mm_free(ptr);
        return NULL;
    }
    else{
        char * nptr=mm_malloc(size);
        char * p=(char *)nptr-SIZE_T_SIZE;
        memcpy((p+SIZE_T_SIZE),ptr,(GETREALSIZE(newptr)-SIZE_T_SIZE));
        mm_free(ptr);
        return (void *)((char *)p+SIZE_T_SIZE);
    }
    return NULL;
}













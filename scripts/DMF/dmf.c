//TODO: not in final
#include <malamute.h>
#include <neon/ne_xml.h>
#include "bestpower-mib.c"
/*
 *      HEADER FILE
 *
 */
#define DEFAULT_CAPACITY 16
typedef enum {
    LOOKUP_LIST = 1,
    INFO_LIST
} type_t;

typedef struct {
	void **values;
	int size;
	int capacity;
	char *name;
	type_t type;
} alist_t;

typedef enum {
    ERR = -1,
    OK
} state_t;

// Create and initialize info_lkp_t
info_lkp_t *
    info_lkp_new (int oid, const char *value);

// Destroy and NULLify the reference to alist_t, list of collections
void
    info_lkp_destroy (info_lkp_t **self_p);

// Create new instance of alist with LOOKUP type, for storage a list of collections
alist_t *
    alist_new ();

/*
 *
 *  C FILE
 *
 */

info_lkp_t *
info_lkp_new (int oid, const char *value)
{
    info_lkp_t *self = (info_lkp_t*) malloc (sizeof (info_lkp_t));
    assert (self);
    memset (self, 0, sizeof (info_lkp_t));
    self->oid_value = oid;
    if(value)
      self->info_value = strdup (value);
    return self;
}

void
info_lkp_destroy (info_lkp_t **self_p)
{
    if (*self_p) {
        info_lkp_t *self = (info_lkp_t*) *self_p;
	printf("Destroying: %d ---> %s\n",self->oid_value, self->info_value);
        if (self->info_value)
	{
            free ((char*)self->info_value);
            self->info_value = NULL;
        }
        free (self);
	*self_p = NULL;
    }
}


alist_t *alist_new (type_t type, const char *name)
{
  alist_t *self = (alist_t*) malloc (sizeof (alist_t));
  assert (self);
  memset (self, 0, sizeof(alist_t));
  self->size = 0;
  self->type = type;
  self->capacity = DEFAULT_CAPACITY;
  self->values = (void**) malloc (self->capacity * sizeof (void*));
  assert (self->values);
  memset (self->values, 0, self->capacity);
  if(name)
    self->name = strdup(name);
  else 
    self->name = NULL;
  return self;
}

void
alist_destroy (alist_t **self_p)
{
    if (*self_p)
    {
        alist_t *self = *self_p;
	
	printf("N elements %d \n",self->size);
	if(self->name)printf("** Name collection: %s \n",self->name);
	
        for (;self->size>0; self->size--){
	  if(self->type == INFO_LIST)
            info_lkp_destroy ((info_lkp_t**)& self->values [self->size-1]);
	  else 
	    alist_destroy ((alist_t**)& self->values [self->size-1]);
	}
	if(self->name)
	  free(self->name);
        free (self->values);
        free (self);
	*self_p = NULL;
    }
}

//Add a generic element at the end of the list
void alist_append(alist_t *self,void *element)
{
  if(self->size==self->capacity)
  {
    self->capacity+=DEFAULT_CAPACITY;
    self->values = (void**) realloc(self->values, self->capacity * sizeof(void*));
  }
    self->values[self->size] = element;
    self->size++;
}

//Return the last element of the list
alist_t *alist_get_last_element(alist_t *self)
{
    return (alist_t*)self->values[self->size-1];
}

int xml_dict_start_cb(void *userdata, int parent,
                      const char *nspace, const char *name,
                      const char **attrs)
{
  alist_t *list = (alist_t*) userdata;
  printf("Node --%s\n", name);
  if(!userdata)return ERR;
  if(strcmp(name,"lookup") == 0)
  {
    alist_append(list, alist_new(INFO_LIST,attrs[1]));
    printf(" %s   Its matched\n",attrs[1]);
  }
  if(strcmp(name,"info") == 0)
  {
    //alist_append((alist_t*)*list->values, info_lkp_new(atoi(attrs[1]), attrs[3]));
    alist_append(alist_get_last_element(list), info_lkp_new(atoi(attrs[1]), attrs[3]));
  }
  return 1;
}

int xml_end_cb(void *userdata, int state, const char *nspace, const char *name)
{
  if(!userdata)return ERR;
  if(strcmp(name,"lookup") == 0)
  {
    printf("Exit function\n");
  }
  return OK;
  
}

int main ()
{
    alist_t * list = alist_new(LOOKUP_LIST,NULL);
    char buffer[1024];
    int result = 0;ne_xml_parser *parser = ne_xml_create ();
    ne_xml_push_handler (parser, xml_dict_start_cb, NULL, xml_end_cb, list);
    FILE *f = fopen ("test.xml", "r");
    if (f) {
        while (!feof (f)) {
            size_t len = fread(buffer, sizeof(char), sizeof(buffer), f);
            if (len == 0) {
                result = 1;
                break;
            } else {
                if ((result = ne_xml_parse (parser, buffer, len))) {
                    break;
                }
            }
        }
        if (!result) ne_xml_parse (parser, buffer, 0);
	/*printf("aqui %s", buffer);*/
        fclose (f);
    } else {
        result = 1;
    }
    ne_xml_destroy (parser);

    printf("Now checking what is in memory and destroying\n");
    alist_destroy(&list);
}
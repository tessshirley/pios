
#include "list.h"

struct list_element *head = &head;

// list_add adds a new node to the beginning of a list
void list_add(struct list_element *list_head, struct list_element *new_element){
    new_element->next = list_head->next;
    list_head->next = new_element;
}

// list_remove removes specific elements from its list
void list_remove(struct list_element *element){
    struct list_element *current = head;

    // travese the list to find the element to remove
    while(current && current->next != element){
	current = current->next;
    }

    // if element is found, remove it from list
    if(current){
	current->next = element->next;
    }
}

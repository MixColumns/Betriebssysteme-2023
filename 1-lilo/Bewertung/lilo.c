// Korrektur: XXXXXXXXXXX
// Punkte: 4 von 4, gut gemacht :)


#include <stdio.h>
#include <stdlib.h>

// Listenstruktur
typedef struct listelement{
    int value; // Zahl die gespeichert wird
    struct listelement *next; // nächstes Listenelement
} listelement;

// Listenkopf ist ein Pointer auf ein Listenelement
static listelement *list = NULL;

static int insertElement(int value) {
    
    // Keine negativen Werte
    if(value < 0){return -1;}
    
    // Temporärer Pointer auf den Wert des Listenkopfes
    listelement *tmp_pointer = list;
    
    // Wenn NULL dann hat die Liste noch keine Elemente und somit auch
    // noch nicht value als Wert
    if(list != NULL){
        // Wenn es nur ein Element gibt wäre ->next NULL darum schauen
        // wir hier vorher noch einmal ob die value in diesem ersten
        // Element bereits vorhanden ist
        if(tmp_pointer->value == value){return -1;}
        // Über alle weiteren Elemente der Liste (falls vorhanden) gehen
        // und schauen ob value schon vorhanden ist, falls ja wie zuvor
        // Fehler zurückgeben. Wenn ->next NULL Ende der Liste erreicht
        while(tmp_pointer->next != NULL){
            // Eins weitergehen
            tmp_pointer = tmp_pointer->next;
            // Nach der value schauen
            if(tmp_pointer->value == value){return -1;}
        }
    }

    // Bisher kein Fehler also ist die value noch nicht in der Liste und
    // positiv. Können es also der Liste hinzufügen. Temporärer pointer
    // auf das neue Element welches mit malloc Speicher der größe eines
    // Elements zugewiesen bekommt.
    listelement *new_Element = (listelement*) malloc(sizeof(listelement));
    // Wenn NULL ist irgendwas schiefgelaufen, Fehler ausgeben
    if(new_Element == NULL){return -1;}
    // Werte setzen aka value eintragen und nächstes Element auf null
    new_Element->value = value;
    new_Element->next = NULL;
    // Wenn es bereits Elemente gibt
    if(list != NULL){
        // Das neue Element anhängen und tmp_pointer (letztes Element)
        // auf das neue Element zeigen lassen
        tmp_pointer->next = new_Element;
    }else{
        // Ansonsten den Listenkopf auf das erste Element zeigen lassen
        list = new_Element;
    }
    
    // Den hinzugefügten Wert zurückgeben
    return value;
}

static int removeElement(void) {
    
    // Wenn die Liste leer ist einen Fehler zurückgeben
    if(list == NULL){return -1;}
    
    // Addresse des zweiten Elements vom Listenkopf aus merken, der Kopf
    // zeigt auf das erste und zeitgleich älteste Element
    listelement *new_addr;
    new_addr = list->next;
    // Den Wert des ersten Elements merken
    int v = list->value;
    // Das erste Element entfernen und speicher frei machen
    free(list);
    // Den Listenkopf auf das zweite Element zeigen lassen
    list = new_addr;
    // Den gemerkten Wert zurückgeben
    return v;
}

int main (int argc, char* argv[]) {
    
    // Noch ein paar extra testcases
    
    printf("insert -2: %d\n", insertElement(-2));
    printf("insert 0: %d\n", insertElement(0));
    printf("insert 47: %d\n", insertElement(47));
    printf("insert 11: %d\n", insertElement(11));
    printf("insert 23: %d\n", insertElement(23));
    printf("insert 11: %d\n", insertElement(11));

    printf("remove: %d\n", removeElement());
    printf("remove: %d\n", removeElement());
    printf("remove: %d\n", removeElement());
    printf("remove: %d\n", removeElement());
    printf("remove: %d\n", removeElement());

    exit(EXIT_SUCCESS);
}
/*P----> +--------------------------------------------------------------------+
         | Punktabzug in dieser Datei: 0 Punkte                               |
         +-------------------------------------------------------------------*/

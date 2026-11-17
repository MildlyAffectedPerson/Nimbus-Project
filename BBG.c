#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_EVENTS 10
#define NUM_TYPES  3

/* ---------- Structures ---------- */
typedef struct {
    char  name[20];
    float price;
} TicketType;

typedef struct {
    int id;
    char name[50];
    int type;      /* index into ticketTypes */
    int seat;      /* allocated seat number  */
    int active;    /* 1 = booked, 0 = cancelled */
} Attendee;

typedef struct WaitNode {          /* waitlist queue node */
    int id;
    char name[50];
    int type;
    struct WaitNode *next;
} WaitNode;

typedef struct {
    char name[50], date[15], venue[50];
    int capacity;
    int booked;
    Attendee *attendees;           /* dynamic array */
    int count, allocated;
    int *seatTaken;                /* dynamic array: 0 free, 1 taken */
    WaitNode *front, *rear;        /* waitlist queue */
} Event;

/* ---------- Globals ---------- */
TicketType ticketTypes[NUM_TYPES] = {
    {"General", 100.0f}, {"Student", 50.0f}, {"VIP", 300.0f}
};
Event events[MAX_EVENTS];
int eventCount = 0;
int nextId = 1;

/* ---------- Helpers ---------- */
int firstFreeSeat(Event *e) {
    for (int i = 0; i < e->capacity; i++)
        if (!e->seatTaken[i]) return i;
    return -1;
}

void addAttendee(Event *e, int id, const char *name, int type, int seat) {
    if (e->count == e->allocated) {            /* grow dynamic array */
        e->allocated *= 2;
        e->attendees = realloc(e->attendees, e->allocated * sizeof(Attendee));
        if (!e->attendees) { printf("Out of memory!\n"); exit(1); }
    }
    Attendee *a = &e->attendees[e->count++];
    a->id = id; strcpy(a->name, name);
    a->type = type; a->seat = seat; a->active = 1;
    e->seatTaken[seat] = 1;
    e->booked++;
}

void enqueue(Event *e, int id, const char *name, int type) {
    WaitNode *n = malloc(sizeof(WaitNode));
    n->id = id; strcpy(n->name, name); n->type = type; n->next = NULL;
    if (e->rear) e->rear->next = n; else e->front = n;
    e->rear = n;
}

/* ---------- Main functions ---------- */
void createEvent(void) {
    if (eventCount == MAX_EVENTS) { printf("Event limit reached.\n"); return; }
    Event *e = &events[eventCount];
    printf("Event name: ");  scanf(" %49[^\n]", e->name);
    printf("Date (DD-MM-YYYY): "); scanf(" %14s", e->date);
    printf("Venue: ");       scanf(" %49[^\n]", e->venue);
    printf("Capacity: ");    scanf("%d", &e->capacity);
    if (e->capacity <= 0) { printf("Invalid capacity.\n"); return; }

    e->booked = 0; e->count = 0; e->allocated = 4;
    e->attendees = malloc(e->allocated * sizeof(Attendee));
    e->seatTaken = calloc(e->capacity, sizeof(int));
    e->front = e->rear = NULL;
    eventCount++;
    printf("Event created with ID %d.\n", eventCount);
}

void listEvents(void) {
    printf("\n%-4s %-25s %-12s %-15s %s\n", "ID", "Name", "Date", "Venue", "Seats left");
    for (int i = 0; i < eventCount; i++)
        printf("%-4d %-25s %-12s %-15s %d/%d\n", i + 1, events[i].name,
               events[i].date, events[i].venue,
               events[i].capacity - events[i].booked, events[i].capacity);
}

void bookTicket(void) {
    int eid, type;
    char name[50];
    listEvents();
    printf("Event ID: "); scanf("%d", &eid);
    if (eid < 1 || eid > eventCount) { printf("Invalid event.\n"); return; }
    Event *e = &events[eid - 1];

    printf("Attendee name: "); scanf(" %49[^\n]", name);
    printf("Ticket type (1=General Rs100, 2=Student Rs50, 3=VIP Rs300): ");
    scanf("%d", &type);
    if (type < 1 || type > NUM_TYPES) { printf("Invalid ticket type.\n"); return; }
    type--;

    int id = nextId++;
    int seat = firstFreeSeat(e);                 /* first-come-first-served */
    if (seat >= 0) {
        addAttendee(e, id, name, type, seat);
        printf("Booked! Booking ID %d, Seat %d.\n", id, seat + 1);
    } else {
        enqueue(e, id, name, type);
        printf("Event full. Added to waitlist. Booking ID %d.\n", id);
    }
}

void cancelTicket(void) {
    int eid, id;
    printf("Event ID: "); scanf("%d", &eid);
    if (eid < 1 || eid > eventCount) { printf("Invalid event.\n"); return; }
    Event *e = &events[eid - 1];
    printf("Booking ID to cancel: "); scanf("%d", &id);

    for (int i = 0; i < e->count; i++) {
        Attendee *a = &e->attendees[i];
        if (a->id == id && a->active) {
            a->active = 0;
            e->seatTaken[a->seat] = 0;
            e->booked--;
            printf("Booking %d cancelled.\n", id);

            if (e->front) {                      /* promote from waitlist */
                WaitNode *w = e->front;
                e->front = w->next;
                if (!e->front) e->rear = NULL;
                int seat = firstFreeSeat(e);
                addAttendee(e, w->id, w->name, w->type, seat);
                printf("Waitlisted %s (ID %d) moved to Seat %d.\n",
                       w->name, w->id, seat + 1);
                free(w);
            }
            return;
        }
    }
    printf("Active booking not found.\n");
}

void report(void) {
    int eid;
    printf("Event ID: "); scanf("%d", &eid);
    if (eid < 1 || eid > eventCount) { printf("Invalid event.\n"); return; }
    Event *e = &events[eid - 1];

    float revenue = 0;
    printf("\n=== %s | %s | %s ===\n", e->name, e->date, e->venue);
    printf("%-5s %-25s %-10s %-5s\n", "ID", "Name", "Ticket", "Seat");
    for (int i = 0; i < e->count; i++) {
        Attendee *a = &e->attendees[i];
        if (!a->active) continue;
        printf("%-5d %-25s %-10s %-5d\n", a->id, a->name,
               ticketTypes[a->type].name, a->seat + 1);
        revenue += ticketTypes[a->type].price;
    }
    printf("\nBooked: %d | Available seats: %d | Revenue: Rs %.2f\n",
           e->booked, e->capacity - e->booked, revenue);

    printf("Waitlist: ");
    if (!e->front) printf("(empty)");
    for (WaitNode *w = e->front; w; w = w->next)
        printf("[%d:%s] ", w->id, w->name);
    printf("\n");
}

void removeEvent(void) {
    int eid;
    char confirm;
    listEvents();
    printf("Event ID to remove: "); scanf("%d", &eid);
    if (eid < 1 || eid > eventCount) { printf("Invalid event.\n"); return; }
    Event *e = &events[eid - 1];

    printf("Remove '%s' with %d booking(s)? All bookings and waitlist will be lost (y/n): ",
           e->name, e->booked);
    scanf(" %c", &confirm);
    if (confirm != 'y' && confirm != 'Y') { printf("Cancelled.\n"); return; }

    /* free this event's dynamic memory */
    free(e->attendees);
    free(e->seatTaken);
    WaitNode *w = e->front;
    while (w) { WaitNode *t = w; w = w->next; free(t); }

    /* shift remaining events down to fill the gap */
    for (int i = eid - 1; i < eventCount - 1; i++)
        events[i] = events[i + 1];
    eventCount--;
    printf("Event removed. Note: remaining event IDs have shifted.\n");
}

void freeAll(void) {
    for (int i = 0; i < eventCount; i++) {
        free(events[i].attendees);
        free(events[i].seatTaken);
        WaitNode *w = events[i].front;
        while (w) { WaitNode *t = w; w = w->next; free(t); }
    }
}

int main(void) {
    int choice;
    do {
        printf("\n--- College Event Ticketing System ---\n"
               "1. Create event\n2. List events\n3. Book ticket\n"
               "4. Cancel ticket\n5. Event report\n6. Remove event\n0. Exit\nChoice: ");
        if (scanf("%d", &choice) != 1) break;
        switch (choice) {
            case 1: createEvent();  break;
            case 2: listEvents();   break;
            case 3: bookTicket();   break;
            case 4: cancelTicket(); break;
            case 5: report();       break;
            case 6: removeEvent();  break;
            case 0: break;
            default: printf("Invalid choice.\n");
        }
    } while (choice != 0);
    freeAll();
    return 0;
}
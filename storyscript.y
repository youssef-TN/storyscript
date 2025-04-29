%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Lexer and parser functions
extern int yylex();
extern int yyparse();
extern FILE *yyin;
void yyerror(const char *s);

// Access to location information
extern int yylineno;  // Current line number from our lexer
extern int column;    // Current column number from our lexer
extern char *yytext;  // Current token text

/* Simple story data structures */
typedef struct Option {
    char *text;
    char *target_room;
    struct Option *next;
} Option;

typedef struct Choice {
    char *text;
    Option *options;
    struct Choice *next;
} Choice;

typedef struct Room {
    char *name;
    char *description;
    Choice *choices;
    struct Room *next;
} Room;

typedef struct Item {
    char *name;
    char *description;
    struct Item *next;
} Item;

typedef struct Story {
    char *title;
    Room *rooms;
    Item *items;
} Story;

/* Global state */
Story *story = NULL;
char current_room[256];
char current_choice[256];
char *current_filename = NULL;  // Track filename for error reporting

/* Function prototypes */
void init_story();
void add_room(const char *name, const char *description);
void add_choice(const char *room_name, const char *choice_text);
void add_option(const char *room_name, const char *choice_text, 
               const char *option_text, const char *target);
void add_item(const char *name, const char *description);
void print_story();
void free_story();
%}

/* Define value types */
%union {
    char *string_val;
}

/* Define tokens */
%token STORY TITLE INVENTORY ITEM ROOM DESCRIPTION CHOICE OPTION GOTO
%token COLON SEMICOLON LBRACE RBRACE
%token <string_val> IDENTIFIER STRING_LITERAL

%%

/* Grammar rules */
story_file: story_definition ;

story_definition:
    STORY LBRACE story_content RBRACE {
        printf("Story parsed successfully\n");
    }
;

story_content: 
    /* empty */
    | story_content story_element 
;

story_element:
    title_def
    | inventory_def
    | room_def
;

title_def:
    TITLE COLON STRING_LITERAL SEMICOLON {
        if (story == NULL) init_story();
        story->title = $3;
    }
;

inventory_def:
    INVENTORY LBRACE inventory_items RBRACE
;

inventory_items:
    /* empty */
    | inventory_items item_def
;

item_def:
    ITEM IDENTIFIER LBRACE item_properties RBRACE {
        free($2); // We're done with the identifier
    }
;

item_properties:
    /* empty */
    | item_properties item_property
;

item_property:
    DESCRIPTION COLON STRING_LITERAL SEMICOLON {
        add_item("current_item", $3); // Using placeholder name
    }
;

room_def:
    ROOM IDENTIFIER {
        strncpy(current_room, $2, sizeof(current_room) - 1);
        free($2);
    } LBRACE room_content RBRACE {
        current_room[0] = '\0'; // Clear current room
    }
;

room_content:
    /* empty */
    | room_content room_element
;

room_element:
    room_description
    | choice_def
;

room_description:
    DESCRIPTION COLON STRING_LITERAL SEMICOLON {
        add_room(current_room, $3);
    }
;

choice_def:
    CHOICE STRING_LITERAL {
        strncpy(current_choice, $2, sizeof(current_choice) - 1);
        add_choice(current_room, $2);
    } LBRACE options RBRACE {
        current_choice[0] = '\0'; // Clear current choice
    }
;

options:
    /* empty */
    | options option_def
;

option_def:
    OPTION STRING_LITERAL GOTO IDENTIFIER SEMICOLON {
        add_option(current_room, current_choice, $2, $4);
    }
;

%%

/* Implementation of core functions */

void init_story() {
    story = (Story *)malloc(sizeof(Story));
    if (!story) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    story->title = NULL;
    story->rooms = NULL;
    story->items = NULL;
}

void add_room(const char *name, const char *description) {
    if (!story) init_story();
    
    Room *room = (Room *)malloc(sizeof(Room));
    if (!room) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    room->name = strdup(name);
    room->description = strdup(description);
    room->choices = NULL;
    
    // Add to the front of the list
    room->next = story->rooms;
    story->rooms = room;
    
    printf("Added room: %s\n", name);
}

void add_choice(const char *room_name, const char *choice_text) {
    if (!story) return;
    
    // Find the room
    Room *room = story->rooms;
    while (room) {
        if (strcmp(room->name, room_name) == 0) {
            // Create new choice
            Choice *choice = (Choice *)malloc(sizeof(Choice));
            if (!choice) {
                fprintf(stderr, "Memory allocation failed\n");
                exit(1);
            }
            
            choice->text = strdup(choice_text);
            choice->options = NULL;
            
            // Add to the front of the list
            choice->next = room->choices;
            room->choices = choice;
            
            printf("Added choice to room %s: %s\n", room_name, choice_text);
            return;
        }
        room = room->next;
    }
    
    fprintf(stderr, "Error at line %d, column %d: Room '%s' not found\n", 
            yylineno, column, room_name);
}

void add_option(const char *room_name, const char *choice_text, 
               const char *option_text, const char *target) {
    if (!story) return;
    
    // Find the room
    Room *room = story->rooms;
    while (room) {
        if (strcmp(room->name, room_name) == 0) {
            // Find the choice
            Choice *choice = room->choices;
            while (choice) {
                if (strcmp(choice->text, choice_text) == 0) {
                    // Create new option
                    Option *option = (Option *)malloc(sizeof(Option));
                    if (!option) {
                        fprintf(stderr, "Memory allocation failed\n");
                        exit(1);
                    }
                    
                    option->text = strdup(option_text);
                    option->target_room = strdup(target);
                    
                    // Add to the front of the list
                    option->next = choice->options;
                    choice->options = option;
                    
                    printf("Added option to go to %s\n", target);
                    return;
                }
                choice = choice->next;
            }
            fprintf(stderr, "Error at line %d, column %d: Choice '%s' not found in room '%s'\n", 
                    yylineno, column, choice_text, room_name);
            return;
        }
        room = room->next;
    }
    
    fprintf(stderr, "Error at line %d, column %d: Room '%s' not found\n", 
            yylineno, column, room_name);
}

void add_item(const char *name, const char *description) {
    if (!story) init_story();
    
    Item *item = (Item *)malloc(sizeof(Item));
    if (!item) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    item->name = strdup(name);
    item->description = strdup(description);
    
    // Add to the front of the list
    item->next = story->items;
    story->items = item;
    
    printf("Added item: %s\n", name);
}

void print_story() {
    if (!story) {
        printf("No story defined\n");
        return;
    }
    
    printf("\n===== STORY =====\n");
    printf("Title: %s\n", story->title ? story->title : "(untitled)");
    
    // Print items
    printf("\n--- ITEMS ---\n");
    Item *item = story->items;
    if (!item) {
        printf("No items defined\n");
    }
    while (item) {
        printf("* %s: %s\n", item->name, item->description);
        item = item->next;
    }
    
    // Print rooms
    printf("\n--- ROOMS ---\n");
    Room *room = story->rooms;
    if (!room) {
        printf("No rooms defined\n");
    }
    while (room) {
        printf("\nROOM: %s\n", room->name);
        printf("Description: %s\n", room->description);
        
        // Print choices
        Choice *choice = room->choices;
        while (choice) {
            printf("  Choice: %s\n", choice->text);
            
            // Print options
            Option *option = choice->options;
            while (option) {
                printf("    Option: %s -> %s\n", 
                       option->text, option->target_room);
                option = option->next;
            }
            choice = choice->next;
        }
        room = room->next;
    }
    
    printf("\n================\n");
}

void free_story() {
    if (!story) return;
    
    // Free items
    Item *item = story->items;
    while (item) {
        Item *next_item = item->next;
        free(item->name);
        free(item->description);
        free(item);
        item = next_item;
    }
    
    // Free rooms
    Room *room = story->rooms;
    while (room) {
        Room *next_room = room->next;
        
        // Free room data
        free(room->name);
        free(room->description);
        
        // Free choices
        Choice *choice = room->choices;
        while (choice) {
            Choice *next_choice = choice->next;
            
            // Free choice data
            free(choice->text);
            
            // Free options
            Option *option = choice->options;
            while (option) {
                Option *next_option = option->next;
                free(option->text);
                free(option->target_room);
                free(option);
                option = next_option;
            }
            
            free(choice);
            choice = next_choice;
        }
        
        free(room);
        room = next_room;
    }
    
    // Free story itself
    free(story->title);
    free(story);
    story = NULL;
    
    // Free filename if allocated
    if (current_filename) {
        free(current_filename);
        current_filename = NULL;
    }
}

int main(int argc, char **argv) {
    // Initialize
    init_story();
    
    // Check for input file
    if (argc > 1) {
        FILE *input = fopen(argv[1], "r");
        if (!input) {
            fprintf(stderr, "Error: Cannot open file '%s'\n", argv[1]);
            return 1;
        }
        yyin = input;
        current_filename = strdup(argv[1]);
    } else {
        printf("Reading from standard input...\n");
        current_filename = strdup("<stdin>");
    }
    
    // Parse input
    yyparse();
    
    // Print and cleanup
    print_story();
    free_story();
    
    return 0;
}

// Enhanced error reporting function
void yyerror(const char *s) {
    fprintf(stderr, "Error in %s at line %d, column %d: %s", 
            current_filename ? current_filename : "<unknown>",
            yylineno, column, s);
    
    // Print the current token if available
    if (yytext && *yytext)
        fprintf(stderr, " near token '%s'", yytext);
    
    fprintf(stderr, "\n");
}
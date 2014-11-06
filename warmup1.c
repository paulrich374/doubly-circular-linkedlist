#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#include<fcntl.h>
#include<unistd.h>
#include<time.h>
#include<math.h>

#include <sys/types.h>
#include"my402list.h"

#ifndef ASCII_ZERO
#define ASCII_ZERO 48
#endif /*ASCII_ZERO*/


typedef struct tagMy402Obj {
	char   type;
	unsigned int time;
	double amount;
	char   description[1024]; 
} My402Obj;

/*----------------- Utility Function --------------------*/
/* command line check message*/
void Usage() {
	fprintf(stderr, "usage: warmup1 sort [tfile]\n");
	exit(-1);
}

/* Print error message and exit out program */
void mal_format (char error_msg[]) {
	printf("error: Malformated input - %s\n ", error_msg);
	exit(1);
}
/* Print My402Obj entity. For debug purpose only */
void PrintObj (My402Obj *obj) {
	printf("Pos/Neg:      |%c| \n" , obj->type);		
	printf("Date:         |%1u|\n" , obj->time);
	printf("Amount:       |%1f|\n" , obj->amount);	
	printf("Description:  |%s| \n" , obj->description);	
}
/* Print My403List entity. For debug purpose only */
void PrintTestList(My402List *list) {
	My402ListElem *temp_elem;
	for ( temp_elem = My402ListFirst(list);  temp_elem != NULL; temp_elem = My402ListNext(list, temp_elem) ) {
		My402Obj *temp_obj = temp_elem->obj;
		printf("Date:         |%1u|\n", temp_obj->time);
		printf("Description:  |%s| \n", temp_obj->description);
		printf("Amount:       |%1f|\n", temp_obj->amount);		
	}	
}
/*----------------- Open a File --------------------*/
/* open a file or read from standard input*/
FILE *OpenFile (int argc, char *argv[]){
	FILE *file;
	int file_descriptor = 0;	
	if ( argc > 3 || argc < 2) Usage(); 
	if ( argc == 3 ) {
		if ( strcmp(argv[1], "sort") != 0 ) Usage();
		if ( argv[2][0] == '-'  ) Usage();
		file_descriptor = 1;
	}
	if ( argc == 2 ) {
		if ( strcmp(argv[1], "sort") != 0 ) Usage();
		file_descriptor = 0;		
	}
	if (file_descriptor == 1) {
		// Open a file
		file = fopen(argv[2], "r"); 
	} else {
		// Scan standard input
		file = stdin;
	}
	if ( !file ) {
		printf("Error: Could not open file\n");
		exit (1);
	}
	return file;
}
/*----------------- Read Input ----------------------*/
/* read input line by line */
void ReadInput(FILE *input, My402List *list) {
	int count_transaction_number = 0;
	char line[80];

	time_t curr_time;

	// read a line
	while ( fgets( line, sizeof(line), input ) ) {
		count_transaction_number++;
		// create a new object	
		My402Obj *obj;
		obj                   = malloc( sizeof(My402Obj) );	
		int count_char        = 0;
		int count_time_digits = 0;
		int count_digits      = 0;		
		int j                 = 0;	
		int k                 = 0;
		// check empty file
		if ( line[count_char] < 32 && count_transaction_number == 1) mal_format("Empty File: Should have at least one transaction.");
		// check end of file
		if ( line[count_char] < 32 ) break;
		// check 1st data: char + or -
		if ( line[count_char] == '+' || line[count_char] == '-' ) {
			obj->type = line[count_char];
			count_char++;
		} else {
			mal_format("No transaction type is found.");
		}
		// check 1st tab: char \t
		if ( line[count_char] == '\t')
			count_char++;
		else
			mal_format("No <TAB> delimeters is found after type string field.");
		// check 2nd data: time
		obj->time = 0;
		curr_time = time( NULL );
		while(1) {
			if ( (line[count_char]) && (line[count_char]) >= '0'  && (line[count_char]) <= '9' ) {
				obj->time = (line[count_char] - ASCII_ZERO) + obj->time*10;
				count_time_digits++;
				count_char++;			
			} else {
				mal_format("No time number is found.");
			}	
			if (line[count_char] == '\n') {
				mal_format("Not suppose to have line break time string field.");
			}
			if (count_time_digits >= 11)
				mal_format("Excessive unsigned integer in the time format.");		
			if (line[count_char] == '\t') {
				break;		
			}
		}// end of while
		if ( obj->time > curr_time ) mal_format("Beyond current time.");
		// check 2nd tab: char \t
		if ( line[count_char] == '\t')
			count_char++;
		else
			mal_format("No <TAB> delimeters is found after time string field.");	
		// check 3nd data: amount		
		obj->amount = 0;		
		while (1) {			
			if (line[count_char] == '.') {
				count_char++;
				break;		
			}
			if ( (line[count_char]) >= '0'  && (line[count_char]) <= '9' ) {
				obj->amount = (line[count_char] - ASCII_ZERO) + obj->amount*10;	
				count_char++;
				count_digits++;	
			} else
				mal_format("No amount number is found.");
			if (count_digits > 8)
				mal_format("Excessive digits in the amount format.");
			if (line[count_char] == '\n') {
				mal_format("Not suppose to not have line break in the amount string field.");
			}	
		}

		for (j = 0; j < 2; j++) {
			if ( (line[count_char]) && (line[count_char]) >= '0'  && (line[count_char]) <= '9' ) {
				obj->amount = (line[count_char] - ASCII_ZERO) + obj->amount*10;
				count_char++;			
			} else
				mal_format("No amount digit is found.");		
		}

		// check 3rd tab: char \t
		if ( line[count_char] == '\t')
			count_char++;
		else
			mal_format("No <TAB> delimeters is found after amount string field..");
		// check 4th data: decription
		while (1) {
			// line longer than 1024 characters
			if (count_char > 1024 ) 
				mal_format("line exceeds 1024 characters.");				
			if (line[count_char] == '\n') {
				obj->description[k] = 0;
				break;
			}
			if (line[count_char] == '\t') 
				mal_format("Not suppose to have tab in the description string field.");

			obj->description[k] = line[count_char];			
			k++;
			count_char++;
		}

		// Insert Obj to the list
		My402ListAppend(list, obj);
		
	}// end of while
	// check transaction number
	if ( count_transaction_number == 0 ) {
				mal_format("Empty File: Should have at least one transaction.");				
	}
}
/*----------------- Bubble Sort --------------------*/
/* sort the list in acending order according to the timestamp*/
void SortList(My402List *list) {
	int number_of_items = My402ListLength(list);
	int m = 0, n = 0;
	My402ListElem *elem_current;
	My402Obj *curr_obj, *next_obj;

	for ( m = 0; m < number_of_items; m++ ) {
		elem_current = My402ListFirst(list);		
		for ( n = 0 ; n < number_of_items - 1; n++) {	
			curr_obj = elem_current->obj;
			next_obj = elem_current->next->obj;
			if ( curr_obj->time == next_obj->time ) {
				mal_format("Same timestamp.");				
			}
			if ( curr_obj->time > next_obj->time ) {
				My402ListInsertBefore(list, next_obj, elem_current);
				My402ListUnlink(list, elem_current->next);
			}
			if ( curr_obj->time < next_obj->time ) {
				// check if next is NULL
				elem_current = elem_current->next;
			}
		}// End of inner for		
	}// End of outer for
}
/* convert amount number to specified string format*/
void AmountConversion (char amount_string[], int amount, char type) {
	int z               = 0;
	char temp_str[15]   = " ?,???,???.?? ";
	amount_string[14]   = 0;
	int amount_value    = amount;
	char type_of_amount = type;

	if ( type_of_amount == '+') {
		amount_string[0] = ' '; amount_string[13] = ' '; 
	} else if ( type_of_amount == '-') {
		amount_string[0] = '('; amount_string[13] = ')'; 
	} 
	if ( amount_value >= 1000000000) {
		strcpy(amount_string, temp_str);
	} else {
		// clear the string first
		for ( z = 1 ; z < 13; z++ ) amount_string[z] = ' ';
		for ( z = 12; z > 0 ; z-- ) {
			if ( z == 2 || z == 6 )
				amount_string[z] = ',';
			else if ( z == 10 )
				amount_string[z] = '.';
			else {
				amount_string[z] = amount_value % 10 + ASCII_ZERO;
				amount_value = amount_value / 10;
				if ( amount_value == 0) break;
			}
		}// end of for	
	} 
}
/*----------------- Print List --------------------*/
/* print out the information of every element in the list in a table manner*/
void PrintList(My402List *list) {
	My402Obj *temp_obj;
	My402ListElem* temp_elem;
	double balance_number = 0;
	char time_string[16];
	char description_string[25];
	char amount_string[15];
	char balance_string[15];
	printf("+-----------------+--------------------------+----------------+----------------+\n");
	printf("|       Date      | Description              |         Amount |        Balance |\n");
	printf("+-----------------+--------------------------+----------------+----------------+\n");

	for( temp_elem = My402ListFirst(list); temp_elem != 0; temp_elem = My402ListNext(list, temp_elem)) {
	temp_obj = temp_elem->obj;
	if (temp_obj->type == '+') {
		balance_number+=temp_obj->amount;
	} else if (temp_obj->type == '-') {
		balance_number-=temp_obj->amount;
	}
	// time conversion to string 
	char *time_temp;
	char *description_temp;
	int x = 0, y = 0;
	time_string[15] = 0; //warning: assignment makes integer from pointer without a cast [enabled by default]
	time_t time_as_time_t = temp_obj->time;
	time_temp = ctime(&time_as_time_t);
	while (x < 15) {
		if ( y < 20 && y > 10 ) { 
			y++;
		} else {
			time_string[x] = time_temp[y];
			x++; y++;
		}
	}
	// description conversion to string 
	x = 0,  y = 0;
	description_string[24] = 0;
	description_temp = temp_obj->description;
	while (x < 24) {
		if (description_temp[y] != 0) {
			description_string[x] = description_temp[y];
			x++; y++;
		}
		if (description_temp[y] == 0) {
			description_string[x] = ' ';
			x++;
		}
	}
	// amount conversion to string
	AmountConversion(amount_string, temp_obj->amount, temp_obj->type);
	// balance conversion to string
	char temp_type = (balance_number >= 0) ?'+':'-';
	AmountConversion(balance_string, abs(balance_number), temp_type);
	
	printf("| ");
	printf("%s | " , time_string);
	printf("%s | " , description_string);
	printf("%s | " , amount_string);	
	printf("%s |"  , balance_string);	
	printf("\n");
	}
	printf("+-----------------+--------------------------+----------------+----------------+\n");
}

/*--------------------- main()  --------------------*/
int main (int argc, char *argv[]) {
	My402List list;
	FILE *in_file; 
	// Open a file
	in_file = OpenFile(argc, argv);
	// Read the input and create a list
	memset(&list, 0, sizeof(My402List));
	// Initialize a list and then it can be a parameter
	if ( !My402ListInit(&list) ) printf("Error: No list existed\n");
	ReadInput(in_file, &list);
	// Sort the list
	SortList(&list);
	// Print the list
	PrintList(&list);
	
	return 0;
}

// #include <string.h>
#include <pebble.h>
// #include <debug.h>
// #include <stdio.h>

// #ifdef _WIN32 /* dynamically linked data needs 'dllimport' */
//  __attribute__((dllimport))
// #endif
// extern Vector3 position;
// extern Vector3 color;
// extern struct Object parent;


#define NB_ITS 1000000
//#define NB_ITS 1
#define TAB_SIZE 100

// int tab[100];
int ret_sum;
char tab3[256];

void *bounds_checking_is_enabled()
{
    char ca[10], *cp = ca-1;
    return (ca != cp + 1) ? cp : NULL;
}

/* error */
int test2(void)
{
	int tab[TAB_SIZE];
    int i, sum = 0;
    for(i=0;i<TAB_SIZE + 1;i++) {
        sum += tab[i];
    }
    return sum;
}

/* ok */
int test4(void)
{
    int i, sum = 0;
    int *tab4;

    tab4 = malloc(20 * sizeof(int));
    for(i=0;i<20;i++) {
        sum += tab4[i];
    }
    free(tab4);

    return sum;
}

int test11(void)
{
    char tab[10];

    memset(tab, 0, 11);
    return 0;
}

// Vector3 position;
// Vector3 color;
// struct Object parent;
// extern struct Object parent;
int *ptr;
void Setup(){
	// Code in here only runs once when this mod is initialized
	// DebugLog(D_ACT, "Calling functions from here!?");
	// ptr = malloc(37);
	// free(ptr);
	// free(ptr);
	// free(ptr);
	// printf("divide by 0?: %d", 145/0);

	// if(__BOUNDS_CHECKING_ON){
		// printf("bounds!\n");
	// }
	// dad
	// bounds_checking_is_enabled();
	// test2();
	// test4();
	// test11();

	printf("Yay?\n");
	// printf("Parent: {%d, %f}\n", parent.d1, parent.f1);
	// printf("Position: {%f, %f, %f}\n", position.x, position.y, position.z);
	// printf("Color: {%f, %f, %f}\n", color.x, color.y, color.z);
}

// static int strlen(char *string){
// 	int i = 0;
// 	while(string[i] != 0){
// 		i++;
// 	}
// 	return i;
// }
#define bool  _Bool
#define false 0
#define true  1

static bool strcmp(char *s1, char *s2){
	int i = 0;
	if(strlen(s1) != strlen(s2)){
		return false;
	}
	while(s1[i] != 0){
		if(s1[i] != s2[i]){
			return false;
		}
		i++;
	}
	return true;
}

void OnClick(UIElement *element){
	// printf("click!: %s\n", element->name);
	// printf("click!");
	if(strcmp(element->name, "color") == 0){
		LoadScene("ui/tests/color.uiss");
	}else if(strcmp(element->name, "layout") == 0){
		LoadScene("ui/tests/layout.uiss");

	}
}

void Loop(){
	// Code in here runs every frame
	// free(ptr);
}
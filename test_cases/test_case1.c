/*
 * Coverage:
 * 1. program
 * 2. declaration-list
 * 3. var-declaration
 * 4. fun-declaration
 * 5. type-specifier
 * 6. params
 * 7. param-list
 * 8. param
 * 9. coumpound-stmt
 * 10. local-declarations
 * 11. statement
 * 12. expression-stmt
 */

int arr[10];
int brr[20];

int proc2(int brr[]){
    /* This is comment and local-declarations test */
    int arr[12];

    /* statement list test */
    ;;;
    brr[1] = 1;
    read arr[2];
    return arr[2] - brr[1];
}

void main(void){
    int num1;
    int num3;
    num1 = 123;
    num3 = proc2(arr);
    write num3;
    return ;
}

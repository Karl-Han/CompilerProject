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

void main(void){
    int num1;
    int num3;
    num1 = 123;
    num3 = 456;
    write num1;
    return ;
}

int proc2(int brr[]){
    /* This is comment and local-declarations test */
    int arr[12];

    /* statement list test */
    ;;;
    proc2(arr);
    arr[1] = 1;
    read arr[2];
    return arr[2] - arr[1];
}

int brr[20];

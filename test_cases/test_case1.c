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

int main(int num1, void id2, int num3){
    num1 = 123;
    num3 = 456;
    return num1 + num3 + 1;
}

void proc1(void)
void proc2(int arr[]){
    /* This is comment and local-declarations test */
    int arr[12];
    void v;

    /* statement list test */
    ;;;
    proc1();
}
{
    return 0;
}

int brr[20];
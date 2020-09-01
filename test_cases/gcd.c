/* A program to perform Euclid's Algorithm to compute gcd. */

int input(void){
    int x;
    read x;
    return x;
}

void output(int x){
    write x;
    return;
}

int gcd(int u, int v)
{
    if (v == 0)
        return u; else return gcd(v, u - u / v * v);
    /*	u-u/v*v == u mod v */
}

void main(void)
{
    int x; int y;
    x = input(); y = input(); output(gcd(x, y));
    return;
}

/* A program to perform selection sort on a 10 element array */

int x[10];

/* find the smallest element in Array a from [low,high] */
int minelement(int arr[], int low, int high){
    int index;
    int smallest_index;
    int smallest;
    index = low + 1;
    smallest_index = low;
    smallest = arr[low];

    while(index <= high){
        if(arr[index] < smallest){
            smallest = arr[index];
            smallest_index = index;
        }

        index = index + 1;
    }
    return smallest_index;
}

/* sort a[low,high] in correct order */
void sort(int arr[], int low, int high){
    int index;
    index = low;
    while(index <= high){
        int temp;
        int smallest_index;

        temp = arr[index];
        smallest_index = minelement(arr, index, high);
        arr[index] = arr[smallest_index];
        arr[smallest_index] = temp;

        index = index + 1;
    }
}

void main(void)
{
    int i;
    i = 0;
    while(i < 10)
    {
        x[i] = input();
        i = i + 1;
    }
    sort(x, 0, 9);
    i = 0;
    while(i < 10)
    {
        output(x[i]);
        i = i + 1;
    }
    return;
}

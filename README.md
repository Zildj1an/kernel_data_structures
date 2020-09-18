# kernel_data_structures

Sort kernel module that showcase the most common kernel data structures (linked list, hash table, red-black tree, radix tree, xarray).


Usage:
```sh
zildj1an@debian$ sudo insmod datas.ko int_str="1,2,3,4,5"
zildj1an@debian$ cat /proc/datas
Linked list: 1, 2, 3, 4, 5
Hash table: 1, 4, 2, 5, 3
Red-black tree: 1, 2, 3, 4, 5
Radix tree: 1, 2, 3, 4, 5
XArray: 1, 2, 3, 4, 5
zildj1an@debian:$
```

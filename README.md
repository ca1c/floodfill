# floodfill

*This program requires the library `libjpeg`*

**Build:**

`$ gcc main.c -ljpeg`

**Run:**

`./a.out <input.jpg> <output.jpg> <newColor>`

Example output:

```
$ ./a.out Cat.jpeg output1.jpg 0xff33f1
Doing floodfill for image Cat.jpeg
New color RGB: 241, 51, 255
Finding most common color...
Most common color RGB: 253, 253, 253
Replacing most common color with new color...
Compressing...
Done! output filename: output1.jpg
```

Original Image:

![cat image](https://raw.githubusercontent.com/ca1c/floodfill/main/Cat.jpeg)

Floodfilled Image (output):

![floodfilled image](https://raw.githubusercontent.com/ca1c/floodfill/main/output1.jpg)
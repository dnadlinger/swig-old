# Operator overloading example
example

a = example.ComplexVal(2,3);
b = example.ComplexVal(-5,10);

printf("a   = %s\n",disp(a));
printf("b   = %s\n",disp(b));

c = a + b;
printf("c   = %s\n",disp(c));
printf("a*b = %s\n",disp(a*b));
printf("a-c = %s\n",disp(a-c));

e = example.ComplexVal(a-c);
printf("e   = %s\n",disp(e));

# Big expression
f = ((a+b)*(c+b*e)) + (-a);
printf("f   = %s\n",disp(f));

# paren overloading
printf("a(3)= %s\n",disp(a(3)));


# kScript
 A custom scripting language written in C++

# Compilation
`g++ --std=c++17 main.cpp -o kScript` or `make`

# Usage (file)
`./kScript example.txt`

# Usage (interpreter)
`./kScript`

# Background
This is a script language compiler and interpreter for a custom language. It supports 
- advanced computational expressions,
- branching with if, elseif and else statements, 
- functions (sqrt, print for example) 
- loops with while
- 27 operators, including unary and binary operators where each operator has a precedence
Internally it uses Reverse Polish Notation to evaluate the statements. It uses the Shunting-yard algorithm to compile
the input into interpretable tokens. 

# Interpreter
Interpreter allows you type in script lines in REPL style and it will output the result of the expression.

# Example

`./kScript example.txt`

Where `example.txt`:

```
print ("5 * 2 / 10 = " + (5 * 2 / 10))

myVariable = 5 * 2 + 1

print myVariable + " is eleven"

strVar1 = "Rob Mayth"
strVar2 = "Is Gonna Rock The Place"
print (strVar1 + " " + strVar2)

i = 0
while (i < 10)
	svYears = "years"
	if (i == 1)
		svYears = "year"
	end
	print ("I am " + i + " " + svYears + " from turning 100")
	i = i + 1
end

condition = true
condition2 = false

if (condition && !condition2)
	print "This will print!"
else
	print "This won't print"
end

index = 0
while (index <= 4)
	if (index == 0)
		print "The moment I wake up"
	elseif (index == 1)
		print "I can feel you right by my side"
	elseif (index == 2)
		print "Knowing you will be around"
	elseif (index == 3)
		print "No matter if I'm wrong or right"
	else
		print "Well I guess I'm stuck with you"
	end
	index = index + 1
end

print("5^2 = " + 5^2)
print("The square root of 7 is " + (sqrt 7))
```

Will output:

```
5 * 2 / 10 = 1
11
Rob Mayth Is Gonna Rock The Place
I am 0 years from turning 100
I am 1 year from turning 100
I am 2 years from turning 100
I am 3 years from turning 100
I am 4 years from turning 100
I am 5 years from turning 100
I am 6 years from turning 100
I am 7 years from turning 100
I am 8 years from turning 100
I am 9 years from turning 100
This will print!
The moment I wake up
I can feel you right by my side
Knowing you will be around
No matter if I'm wrong or right
Well I guess I'm stuck with you
5^2 = 25
The square root of 7 is 2.6457513
```
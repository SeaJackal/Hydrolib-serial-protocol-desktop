# Serial protocol utility

Linux master hydrolib serial protocol utility for using with corresponding 
Hydrolib-soft module on the slave device (https://github.com/SeaJackal/Hydrolib-soft).

## Building

To build application call from project root directory:
```
cmake -B ./build
cmake --build ./build
```

## Usage

To show common help call from project root directory
```
./build/hydrosp -h
```
And for command specific help call
```
./build/hydrosp <command> -h
```

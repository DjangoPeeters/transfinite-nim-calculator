# transfinite-nim-calculator

This is a C++ port of a program for calculating with transfinite nimbers, available in cgsuite and written in Scala, licensed under the GNU GPL v3.0 by Aaron Siegel.

## License

This project is licensed under the GNU General Public License v3.0. See the [LICENSE](LICENSE) file for details.

## Author

Django Peeters

## Usage

Currently, there is only support for calculating Lenstra excess and converting the results to an a-file and a b-file for the OEIS. When calculating Lenstra excess, some other values are calculated as well (Q-sets, the nimbers alpha(p), degree of kappa(p)). Following are some examples of these features.

First and foremost, you can just let the program calculate the nimbers alpha(p) in order, starting from p=3:
```
./bin/main alphas
```
This will store values inside the `logs` folder. If you want to be more specific about where to keep these log-files (or just to be sure) you can enter the relative path to the folder where the logs should be kept. For example:
```
./bin/main alphas logs
```
If you let this run for long enough, you will encounter an Out Of Memory error because the calculations need more space. This is normal behavior. To this end, you can specify how big a calculation may go depending on the exponent of the impartial term algebras (fields of order 2^n). The default value is 1000000 (a million, for practical purposes). For example:
```
./bin/main alphas logs 3000000
```

You can also let the program calculate a specific alpha(p), for example:
```
./bin/main alpha logs 47
```
For convenience, you can also specify which prime using their index (nth_prime(2) = 3, nth_prime(3) = 5, etc.):
```
./bin/main alpha logs nth_prime 15
```
There's also the option to just leave this information out. In this case, the program will calculate the first unknown alpha(p) (unknown to your copy of the program, to update this information you can just pull from this repository). For example:
```
./bin/main alpha logs
```
or
```
./bin/main alpha
```

Last but not least, you can convert the stored Lenstra excess (inside `logs/excess_records.txt`) to an a-file for OEIS:
```
./bin/main afile
```
or
```
./bin/main afile logs
```
and analogously to a b-file:
```
./bin/main bfile
```
or
```
./bin/main bfile logs
```

That's it!

# transfinite-nim-calculator

This is a C++ port of a program for calculating with transfinite nimbers, available in cgsuite and written in Scala, licensed under the GNU GPL v3.0 by Aaron Siegel.

## License

This project is licensed under the GNU General Public License v3.0. See the [LICENSE](LICENSE) file for details.

## Author

Django Peeters

## Usage

Currently, there is only support for calculating Lenstra excess and converting the results to an a-file and a b-file for the [OEIS](https://oeis.org/A380496). When calculating Lenstra excess, some other values are calculated as well (Q-sets, the nimbers alpha(p), degree of kappa(p)). Following are some examples of these features.

Every invocation has the same shape:
```
./bin/main LOGS_DIR COMMAND [args...]
```
`LOGS_DIR` is the relative path to the folder where log/cache files are kept (created automatically if it doesn't exist yet) — it comes first since it isn't specific to any one command. `COMMAND` is one of `alphas`, `alpha`, `afile`, `bfile`, `calc` (below).

First and foremost, you can just let the program calculate the nimbers alpha(p) in order, starting from p=3:
```
./bin/main logs alphas
```
If you let this run for long enough, you will encounter an Out Of Memory error because the calculations need more space. This is normal behavior. To this end, you can specify how big a calculation may go depending on the exponent of the impartial term algebras (fields of order 2^n). The default value is 1000000 (a million, for practical purposes). For example:
```
./bin/main logs alphas 3000000
```
If you're running this on a server with a wall-time cap (e.g. a 72-hour job limit), you can chunk a sweep across multiple jobs by giving a starting prime as a second argument to `alphas`, either directly or by index (same `nth_prime` convention as below):
```
./bin/main logs alphas 3000000 173
```
```
./bin/main logs alphas 3000000 nth_prime 40
```

You can also let the program calculate a specific alpha(p), for example:
```
./bin/main logs alpha 47
```
For convenience, you can also specify which prime using their index (nth_prime(2) = 3, nth_prime(3) = 5, etc.):
```
./bin/main logs alpha nth_prime 15
```
There's also the option to just leave this information out. In this case, the program will calculate the first unknown alpha(p) (unknown to your copy of the program, to update this information you can just pull from this repository). For example:
```
./bin/main logs alpha
```
IMPORTANT NOTE: the current implementation only works for primes smaller than 12289, the 1470-th prime. (This is because the Q-set of 12289 would be {2048, 3}, and this leads to a finite summand not fitting in a `uint256_t`.)

Last but not least, you can convert the stored Lenstra excess (inside `logs/excess_records.txt`) to an a-file for OEIS:
```
./bin/main logs afile
```
and analogously to a b-file:
```
./bin/main logs bfile
```

## Calculating with nimbers

Beyond computing individual alpha(p) values, you can evaluate ordinal/nimber arithmetic expressions directly:
```
./bin/main logs calc "w^3 + w*2 + 1"
```
`+`, `*` and `w^E` are ordinal (Cantor normal form) arithmetic — the same non-commutative sum/product used to write ordinals below w^(w^w) in normal form (so e.g. `2 * w` gives `w`, not `w*2`, and `(w+1) * 2` gives `w*2 + 1`).

`+.`, `*.` and `^.` are nim (field) arithmetic instead — nim-addition, nim-multiplication, and nim-exponentiation (with a plain non-negative integer exponent), the operations alpha(p) exists to support. For example:
```
./bin/main logs calc "w +. w"     # nim-add: always 0 for any value +. itself
./bin/main logs calc "w *. w"     # nim-multiply: w^2
./bin/main logs calc "w ^. 5"     # nim-power
```
Large enough nim-multiplications/-powers will compute alpha(p) values on demand behind the scenes, same as the `alpha`/`alphas` commands, and are subject to the same prime-size limitation noted above.

A bare (unparenthesized) exponent after `w^` can only be `w` itself or a plain integer, since exponentiation binds tighter than `+`/`*` — anything more needs explicit parentheses, e.g. `w^(w*2)` or `w^(w^2 + 3)`.

That's it!

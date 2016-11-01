# Synopsis

fairdice is a very simple C program to check for dice fairness.

Usage:

    fairdice <num-sides> < rolls.txt

# The tests

* A
  [ECDF](https://en.wikipedia.org/wiki/Empirical_distribution_function)
  test, similar to [Kolmogorov-Smirnov goodness-of-fit
  test](https://en.wikipedia.org/wiki/Kolmogorov%E2%80%93Smirnov_test),
  but adapted to discrete dice. The ideal CDF with fair dice is
  randomly generated using the Monte-Carlo method. **This method is
  more accurate for dice with lots of sides, where the goal is usually
  to beat a certain value.**
  
* A regular [chi-square
  test](https://en.wikipedia.org/wiki/Pearson%27s_chi-squared_test). Thanks
  to the [GNU Scientific Library](https://www.gnu.org/software/gsl/)
  for doing the heavy math. **This method will test against any kind
  of skewness but is less powerful on large dice.**

* Finally, check observed frequencies and their 99% confidence
  interval.


# Examples

    fairdice 6 < examples/syg-d6-blue.txt
    
    SmpSize:   n=180
    ECDF:      p=0.7500
    ChiSq:     p=0.8583
    ConfInt99: OK

This dice is very likely fair. Increase the sample size if you need to
be more sure.

***

    fairdice 20 < examples/syg-d20-blue.txt

    SmpSize:   n=1000
    ECDF:      p=0.6563
    ChiSq:     p=0.6943
    ConfInt99: OK

This dice is very likely fair.

***

    fairdice 20 < examples/syg-d20-green.txt
    
    SmpSize:   n=1000
    ECDF:      p=0.1407
    ChiSq:     p=0.1336
    ConfInt99: 9+ 18-

This dice looks problematic, two sides are not in their confidence
intervals. Requires further testing.

***

    fairdice 20 < examples/syg-d20-black.txt

    SmpSize:   n=1000
    ECDF:      p=0.1875
    ChiSq:     p=0.0042
    ConfInt99: 12-

This dice is probably unfair according to the chi-square test, but not
according to the K-S test. Requires further testing.

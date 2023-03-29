# Ride Sharing

An __UNOFFICIAL__ implementation of paper [1].

Not under active maintenance.

This project was developed at a time when there were no open-source implementations available.
In recent years, several community-maintained projects supporting the algorithm proposed in this paper have emerged, which can be found by searching for the paper title on GitHub. It is recommended that those interested turn to these projects for a better experience.

### Dependencies

- metis
- Gurobi

### Usage

```
make
./Main <requests_file> <vehicles_file> <output_file> <max_capacity>
```

### Data

Please see [this issue](https://github.com/MetaZuo/RideSharing/issues/1).


### References

[1] Alonso-Mora J, Samaranayake S, Wallar A, et al. On-demand high-capacity ride-sharing via dynamic trip-vehicle assignment[J]. Proceedings of the National Academy of Sciences, 2017, 114(3): 462-467.

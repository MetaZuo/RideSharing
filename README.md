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

### About GP-Tree

`GP_Tree.data` is the cached GP-Tree data structure for the NYC road network. This tree is used for efficiently querying shortest paths between arbitrary pairs of nodes on road networks. GP-Tree is an improved version of [G-Tree (paper link)](https://dbgroup.cs.tsinghua.edu.cn/ligl/papers/tkde15-gtree.pdf) and the predecessor of [V-Tree (paper link)](https://dbgroup.cs.tsinghua.edu.cn/ligl/papers/icde17-vtree.pdf). I myself know little about this line of work, whereas a member in my team was one of the authors and provided us with the source code `GPtree.cpp`.  Refer to the papers for details if needed, and you can find official implementations of the trees on GitHub: [G-Tree (& GP-Tree)](https://github.com/TsinghuaDatabaseGroup/GTree/tree/master/src/gtree_new_p2p) and [V-Tree](https://github.com/TsinghuaDatabaseGroup/VTree).

`Main.cpp` invokes the function `void initialize(bool load_cache)` from `GPtree.cpp`, with the argument `true` to load the already-built data structure `GP_Tree.data` to memory. __If you want to work on another road network instead of the NYC, please 1) backup `COL.co` and `cal.edge` and then replace them with the corresponding node and edge files of your road network but keep the file names (sorry for the hard-coding), and 2) replace the invocation with `initialize(false)` for the first run.__ A new tree will be built from your road network and cached to `GP_Tree.data`. Once the cache file is built, you can change back to `initialize(true)` to avoid building the tree again.

See https://github.com/MetaZuo/RideSharing/issues/1#issuecomment-1490077275


### References

[1] Alonso-Mora J, Samaranayake S, Wallar A, et al. On-demand high-capacity ride-sharing via dynamic trip-vehicle assignment[J]. Proceedings of the National Academy of Sciences, 2017, 114(3): 462-467.

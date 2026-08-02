# RushHourDataset

The data repository contains the following files:

*   **part(1-12).txt**: Raw text containing the data of $30,233,088$ clusters each for a total of $362,797,056$ clusters. (**Note:** around **8 gigabytes**
of disk space is required to fully clone the dataset).
*   **retrograde_analysis**: Folder containg the C++ source code for an algorithm which takes a cluster, calculates the desired metric value (solution length, solution entropy, or a normalized combination of both) for all the boards within the cluster, and prints the board in descending order in a readable ASCII format.

The data files contain lines with the following information about the cluster:


*   **Cluster identifier**: A hexadecimal value serving as the identifier of the cluster. The number of pieces and their distribution can be derived from this identifier through an unranking process, exemplified as follows for cluster `0x504d711c`:

    1. Convert the identifier to a base-6 number. i.e., `0x504d711c` written in base-6 is `341404144044`.
    2. Each digit of this base-6 corresponds to one of the distribution type indices shown in Figure 2. The first 6 digits correspond to the distribution type of the columns, while the remaining 6 digits correspond to the distribution type of the rows. Therefore, the cluster `0x504d711c` is populated by all boards following the column distribution `[3,4,1,4,0,4]` and the rows with distribution `[1,4,4,0,4,4]`.

* **Legal**: A binary identifier that describes whether a cluster has boards which can be constructed with a physical version of the game (does not exceed the allowed number of pieces and there are boards where the pieces do not overlap with each other).

* **Has Solution**: A binary identifier describing whether a cluster has at least one board within it that has a solution. **Note:** A cluster can have multiple solutions within it, this only identifies if there is at least one.

* **Cluster Size**: The number of boards that can be constructed.

* **Max Length**: The value of the board(s) within the cluster which is the farthest from a solution. I.e., a cluster with solution length 10 has at least one board requiring a minimum of 10 moves to solve, which is the maximum locally.

* **Time**: The time it took, in seconds, to run `retrograde_analysis` locally when analyzing the solution length, using a machine wish specifications: ``OS: Ubuntu 24.04.4 (WSL), CPU: 8-core AMD Ryzen 7 6800H, RAM: 16GB``
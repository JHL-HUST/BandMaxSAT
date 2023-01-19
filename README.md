BandMaxSAT: A Local Search MaxSAT Solver with Multi-armed Bandit
----
This repository contains the code to the BandMaxSAT algorithm for the MaxSAT proposed in our paper: <br> <br>
[BandMaxSAT: A Local Search MaxSAT Solver with Multi-armed Bandit (IJCAI 2022)](https://www.ijcai.org/proceedings/2022/264) <br>
Jiongzhi Zheng, Kun He, Jianrong Zhou, Yan Jin, Chu-Min Li, Felip Manya <br> <br>

Installation
----
On a Unix/Linux machine execute the following commands: <br> <br>

unzip BandMaxSAT-main.zip <br>
cd BandMaxSAT-main <br>
make <br> <br>

An executable file called BandMaxSAT will now be available in the directory BandMaxSAT-main. <br>
Then enter the command ./BandMaxSAT instance_name, to run the algorithm. <br> <br>

For BandMaxSAT-c, we refer to the DT-HyWalk solver submitted to MSE 2022 (available at https://maxsat-evaluations.github.io/2022/mse22-solver-src/incomplete/DT-HyWalk.zip). Fix the variable selected_strategy to 3 in files MaxSAT.cc and Alg_LinearSU_Clustering.cc in DT-HyWalk results in an implementation of BandMaxSAT-c. <br> <br>

Contact
----
Questions and suggestions can be sent to jzzheng@hust.edu.cn. <br> <br>

Citation
----
If you find this code useful, please consider citing the original work by authors: <br>
```
@inproceedings{zheng2022BandMaxSAT,
  title={BandMaxSAT: A Local Search MaxSAT Solver with Multi-armed Bandit},
  author={Jiongzhi Zheng and Kun He and Jianrong Zhou and Yan Jin and Chu-Min Li and Felip Manya},
  booktitle={The Thirty-First International Joint Conference on Artificial Intelligence},
  page={1901--1907},
  year={2022}
}
```

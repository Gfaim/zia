# Zia

## :book: Summary
  - [:book: Summary](#book-summary)
  - [:warning: Requirement](#warning-requirement)
  - [:computer: Usage](#computer-usage)
    - [:rocket: Clone repository](#rocket-clone-repository)
    - [:wrench: Setup repository](#wrench-setup-repository)
  - [:hammer: Build](#hammer-build)
  - [:bust_in_silhouette: Authors](#bust_in_silhouette-authors)

## :warning: Requirement

- [CMake >= 3.17](https://cmake.org/download/)
- [C++20](https://en.cppreference.com/w/cpp/20)

## :computer: Usage

### :rocket: Clone repository

```sh
git clone git@github.com:EpitechPromo2024/B-YEP-500-PAR-5-1-zia-diego.rojas.git
```

### :wrench: Setup repository

```sh
pip install conan
conan profile new default --detect
conan remote add bincrafters https://bincrafters.jfrog.io/artifactory/api/conan/public-conan
conan config set general.revisions_enabled=1
```

## :hammer: Build

Using Makefile:
```sh
# to build the program
make
./zia

# to build the tests
make tests
./unit_tests
```

Using CMake:
```sh
# to build the program
conan install . -if build --build=missing
cmake . -B build/
cmake --build build/
./zia

# to build the tests
conan install . -if build_tests --build=missing
cmake . -B build_tests/ -DZIA_BUILD_TESTS=ON
cmake --build build_tests/
./unit_tests
```

## :bust_in_silhouette: Authors

 - [Martin Olivier](https://github.com/martin-olivier)
 - [Diego Rojas](https://github.com/rojasdiegopro)
 - [Edouard Sengeissen](https://github.com/edouard-sn)
 - [Nicolas Allain](https://github.com/Nirasak)
 - [Romain Minguet](https://github.com/Romain-1)
 - [Allan Debeve](https://github.com/Gfaim)

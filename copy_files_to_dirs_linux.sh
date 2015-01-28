 #!/bin/bash
IFS=$'\n' read -d '' -r -a files < $1
IFS=$'\n' read -d '' -r -a dirs < $2

cd $3
for i in ${!files[@]}; do
  cmake -E make_directory ${dirs[$i]}
  cmake -E copy_if_different ${files[$i]} ${dirs[$i]}
done

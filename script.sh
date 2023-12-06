if [ $# -ne 1 ]
then
    echo "Eroare"
    exit 1
fi
 
caracter="$1"
count=0 

while IFS= read -r line
do 
  if echo "$line" | egrep "^[A-Z][A-Za-z0-9,?! ]+[.|?|!]$" | grep -v ",si" > /dev/null
then
    ((count++))
fi
done 

echo "Counterul are valoarea: $count"

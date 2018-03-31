red=$'\e[1;31m'
grn=$'\e[1;32m'
end=$'\e[0m'

curl "http://localhost:8081/" > /dev/null 2> /dev/null

[[ $? == 0 ]] && printf "%s\n" "${grn}PASS${end}" || printf "%s\n" "${red}FAIL${end}"

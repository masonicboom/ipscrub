red=$'\e[1;31m'
grn=$'\e[1;32m'
end=$'\e[0m'

curl -6 "http://localhost:8081/" > /dev/null 2> /dev/null
output6=$(tail -n 1 dest/logs/access.log)

curl -4 "http://localhost:8081/" > /dev/null 2> /dev/null
output4=$(tail -n 1 dest/logs/access.log)

[[ "${#output6}" == 22 && "${#output4}" == 6 ]] && printf "%s\n" "${grn}PASS${end}" || printf "%s\n" "${red}FAIL${end}"

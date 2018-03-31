red=$'\e[1;31m'
grn=$'\e[1;32m'
end=$'\e[0m'

salt="SALT"
input="abcdef"

curl "http://localhost:8081/$salt$input" > /dev/null 2> /dev/null
output=$(tail -n 1 dest/logs/access.log)

# -n excludes newline
# cut -c-40 takes first 40 chars (shasum outputs 160 bits in hex)
# xxd converts hex to binary
# base64 converts binary to ascii
clihash=$(echo -n "$input$salt" | shasum | cut -c-40 | xxd -r -p | base64)

[[ $clihash == $output ]] && printf "%s\n" "${grn}PASS${end}" || printf "%s\n" "${red}FAIL${end}"

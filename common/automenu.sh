#!/bin/bash

####### MAGIC BELOW ######
if test -f ${BASH_SOURCE[0]%/*}/../../system-settings.env; then
	set -o allexport
	source ${BASH_SOURCE[0]%/*}/../../system-settings.env
	set +o allexport
else
	echo "!! WARNING !! : Project wide system settings are not set ::"
fi

unameOut="$(uname -s)"
case "${unameOut}" in
	Linux*)     HOST_OS=Linux;;
	Darwin*)    HOST_OS=Mac;;
	CYGWIN*)    HOST_OS=Cygwin;;
	MINGW*)     HOST_OS=MinGw;;
	*)          HOST_OS="UNKNOWN:${unameOut}"
esac
export HOST_OS=${HOST_OS}

root_description="go back to root runme if it exists"
root()
{
    FULLPATH=${BASH_SOURCE[0]}
    ROOTPATH=${FULLPATH%/*}/
    echo $ROOTPATH
	cd $ROOTPATH/../.. && ./runme.sh ${@:1}
}

Help_description="this functions prints out all the function names and their description if they have one"
Help()
{
	index=1
	for function in "${functions_list[@]}"
	do
		description=${functions_list[$index]}\_description
		echo -e $function'\t': ${!description}'\n'
		((index++))
	done
}

i=1 #Index counter for adding to array
j=1 #Option menu value generator
functin_index=1
declare -a functions_list
declare -a options
declare -a descriptions

IFS=$'\n'
for f in $(declare -F); do
	functions_list[ $functin_index]="${f:11}"
	description=${functions_list[$functin_index]}\_description
	descriptions[$functin_index]="${!description}"
	(( functin_index+=1 ))
	options[ $i ]=$j
	(( j+=1 ))
	options[ ($i + 1) ]="${f:11}""    : ""${!description}"
	(( i=($i+2) ))
done

apt_check_and_install_description="checks if an app exists and apt install if the name is the same as the apt name"
apt_check_and_install() 
{
	if ! command -v "$1" > /dev/null 2>&1; then
		sudo apt-get install "$1" -y
	fi
} 

apt_check_and_install "dialog"

menu_description="this will render the dialog based on all the functions that exist in the scripts"
menu()
{
	
	cmd=(dialog --keep-tite --menu "Select options:" 32 140 26)
	choices=$("${cmd[@]}" "${options[@]}" 2>&1 >/dev/tty)

	for choice in $choices
	do
		case $choice in
		*[!\ ]*)
			description=${functions_list[$choice]}\_description
			echo ${!description}
			${functions_list[$choice]}
		;;
		esac
	done
}
# printf '%s\n' "${0##*/}"
# echo "arugments given are : ${@:1}"
# echo "Params to pass : ${@:2}"

command=$(echo ${@:1} | cut -d' ' -f1)
# echo "Command to use is : $command"

case $1 in
*[!\ ]*)
		${command} ${@:2}
		# ${@:1} # Use this command if issues
	;;
*)
	menu $@
	;;
esac


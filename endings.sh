for file in *; do
    # echo $file
    tempfile=$file-temp
    sed -e "s/\r//" $file > $tempfile
    mv $file $file-toremove
    mv $tempfile $file
done

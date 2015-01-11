#!/bin/sh

tagflush()
{
	find $(pwd) -name "*.h" -o -name "*.c" -o -name "*.cc" -o -name "*.hpp" -o -name "*.cpp" -o -name "*.java" -o -name "*.dd" |sed 's/^\(.*\)$/"\1"/g' > cscope.files
#find $(pwd) -name "*.java"|sed 's/^\(.*\)$/"\1"/g' > cscope.files
		cscope -bkq -i cscope.files
		ctags -R
}

cd device/mediatek_common/vm_linux/chiling/app_if && 
{
	tagflush
	echo tags for "$(pwd)" ok
	#ln -s `pwd`/cscope.out ~/cscope_app_if.out
	cd -
	#[ -f cscope_app_if.out ] || mv ~/cscope_app_if.out .
}


cd device/tpvision/common/plf/tpapi &&
{
	tagflush
	echo tags for "$(pwd)" ok
	#ln -s `pwd`/cscope.out ~/cscope_tpapi_if.out
	cd -
	#[ -f cscope_tpapi_if.out ] || mv ~/cscope_tpapi_if.out .
}

cd device/tpvision/tvsoc/tvsoc_mtk &&
{
	tagflush
	echo tags for "$(pwd)" ok
	#ln -s `pwd`/cscope.out ~/cscope_tpapi_src.out
	cd -
	#[ -f cscope_tpapi_src.out ] || mv ~/cscope_tpapi_src.out .
}

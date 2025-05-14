# This Script reset dali shader in target

FILE_NAME="shader.versionstamp"
DALI_SHADER_VERSION_PATH="/usr/share/dali/core/shaderbin/"
DALI_SHADER_VERSION_FILE="${DALI_SHADER_VERSION_PATH}/${FILE_NAME}"
DALI_SHADER_CACHE_VERSION_PATH=/home/owner/.cache/dali_common_caches/
DALI_SHADER_CACHE_VERSION_FILE="${DALI_SHADER_CACHE_VERSION_PATH}/${FILE_NAME}"

function get_dali_shader_version(){
    if [ -e ${DALI_SHADER_VERSION_FILE} ]
    then
        _dali_shader_version=`cat ${DALI_SHADER_VERSION_FILE}`".version"
    else
        _dali_shader_version="Unknown.version"
    fi

    eval "$1='${_dali_shader_version}'"
}

function get_dali_shader_cache_version(){
     if [ -e ${DALI_SHADER_CACHE_VERSION_FILE} ]
     then
         _dali_shader_cache_version=`cat ${DALI_SHADER_CACHE_VERSION_FILE}`".version"
     else
         _dali_shader_cache_version="Unknown.version"
     fi

     eval "$1='${_dali_shader_cache_version}'"
}

get_dali_shader_version _current_dali_shader_version
get_dali_shader_cache_version _current_dali_shader_cache_version

rm -rf ${DALI_SHADER_CACHE_VERSION_PATH}/shader/*.tmp  #delete tmp file every booting

if [ -e ${DALI_SHADER_CACHE_VERSION_FILE} ]
then
    if [ ${_current_dali_shader_version} != ${_current_dali_shader_cache_version} ]
    then
        echo "dali version: ${_current_dali_shader_version}"
        echo "cache version: ${_current_dali_shader_cache_version}"
        rm -rf ${DALI_SHADER_CACHE_VERSION_PATH}shader/
        cp -R ${DALI_SHADER_VERSION_FILE} ${DALI_SHADER_CACHE_VERSION_FILE} # version sync
        echo "remove shader cache"
    fi
fi

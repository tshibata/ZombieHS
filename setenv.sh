pushd `dirname $0`
export COCOS_CONSOLE_ROOT="`pwd`/cocos2d/templates"
export COCOS_TEMPLATES_ROOT="`pwd`/cocos2d/tools/cocos2d-console/bin"
export PATH=$COCOS_TEMPLATES_ROOT:$COCOS_CONSOLE_ROOT:$PATH
popd

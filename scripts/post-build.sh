#!/bin/sh

# copy over shaders
cp -r ../resources $out

# wrap binary so alsa can find its plugins
mkdir -p $out/bin
mv ./visua $out/bin/_visua

echo "#!/bin/sh" > $out/bin/visua
echo "ALSA_PLUGIN_DIR=$ALSA_PLUGIN_DIR VISUA_RESOURCES_PATH=$out/resources nixGL $out/bin/_visua" >> $out/bin/visua
chmod a+x $out/bin/visua

echo "#!/bin/sh" > $out/bin/visua-debug
echo "ALSA_PLUGIN_DIR=$ALSA_PLUGIN_DIR VISUA_RESOURCES_PATH=$out/resources nixGL gdb $out/bin/_visua" >> $out/bin/visua-debug
chmod a+x $out/bin/visua-debug

#!/bin/sh

# copy over shaders
cp -r ../resources $out/resources

# wrap binary so alsa can find its plugins
mkdir -p $out/bin
mv ./visua $out/bin/_visua
echo "#!/bin/sh" > $out/bin/visua
echo "ALSA_PLUGIN_DIR=$ALSA_PLUGIN_DIR VISUA_RESOURCES_PATH=$out/resources nixGL $out/bin/_visua" >> $out/bin/visua
chmod a+x $out/bin/visua

#!/bin/bash
pushd ../..

if [ x"$#" = x1 ]; then
  build=`gawk -- '$2 ~ "VERSION_BUILD" {print $3}' copasi/copasiversion.h`

  case x"$1" in 
  xWIN32)
    zip CopasiUI-$build-$1.zip CopasiUI.exe 
    ;;

  xDarwin)
    if otool -L ./copasi/CopasiUI/CopasiUI.app/Contents/MacOS/CopasiUI \
      | grep libqt 2>&1 > /dev/null ;then
        echo ""
        echo "CopasiUI is dynamicall linked against Qt."
        echo "Please link the binary statically before making an image."
        echo ""
        exit 1
    fi
    
    SETFILE=/Developer/Tools/SetFile

# Create temporary package image drive
    hdiutil create Copasi-tmp.dmg -megabytes 200 -volname Copasi-$build-$1 -fs HFS+ \
      -layout NONE

# Mount temporary package image drive
    drive=`hdid Copasi-tmp.dmg | gawk -- '{print $1}'`
    TMPDIR=`df -l | gawk -- '$1 ~ "'$drive'" {print $6}'`
    echo $drive '->' $TMPDIR

    echo "Copying application bundle."
    mkdir ${TMPDIR}/copasi
    cp -r copasi/CopasiUI/CopasiUI.app ${TMPDIR}/copasi/

    strip ${TMPDIR}/copasi/CopasiUI.app/Contents/MacOS/CopasiUI

# copy the icon into the Resources directory
    echo "Creating Resources directory."
    mkdir -p ${TMPDIR}/copasi/CopasiUI.app/Contents/Resources
    echo "Copy the icon file to the bundle."
    cp copasi.icns ${TMPDIR}/copasi/CopasiUI.app/Contents/Resources/

# set the icon in the Info.plist file in the Contents directory
# the icon file name is located between <string></string> tags in the line after
# <key>CFBundleIconFile</key>
# Maybe this would better be done with a dom parser.
echo "Set the icon in the Info.plist file."
    LINE=`grep -n "CFBundleIconFile" ${TMPDIR}/copasi/CopasiUI.app/Contents/Info.plist \
      | cut -c1`
    LINE=$((LINE+1))
    sed -e "${LINE}s%<string>.*</string>%<string>copasi.icns</string>%" \
      ${TMPDIR}/copasi/CopasiUI.app/Contents/Info.plist > ${TMPDIR}/tmp.plist
    mv ${TMPDIR}/tmp.plist ${TMPDIR}/copasi/CopasiUI.app/Contents/Info.plist

# copy the examples into the Resources directory
    echo "Make example directory."
    mkdir -p ${TMPDIR}/copasi/CopasiUI.app/Contents/Resources/examples
    echo "Copy examples to example directory."
    for EXAMPLE in $EXAMPLES;do
        cp ${EXAMPLE} ${TMPDIR}/copasi/CopasiUI.app/Contents/Resources/examples/
    done

# copy the files for the wizard into the Resources directory
    echo "Making directory for wizard."
    mkdir -p ${TMPDIR}/copasi/CopasiUI.app/Contents/Resources/doc/html
    echo "Copying files for wizard."
    cp ./copasi/wizard/help_html/*.html \
      ${TMPDIR}/copasi/CopasiUI.app/Contents/Resources/doc/html/
    cp -r ./copasi/wizard/help_html/figures \
      ${TMPDIR}/copasi/CopasiUI.app/Contents/Resources/doc/html/
# add the readme to the image
    echo "Copying readme file."
    cp README_MAC.rtf ${TMPDIR}/copasi/COPASI-README.rtf

# set the bundles flag with SetFile
    echo "Setting the bundle flag."
    ${SETFILE} -a b ${TMPDIR}/copasi/CopasiUI.app

    chown -R 99:99 ${TMPDIR}/copasi

# Unmount temporary image
    hdiutil eject ${drive}

    hdiutil convert -format UDCO Copasi-tmp.dmg -o Copasi-$build-$1.dmg
    rm -rf Copasi-tmp.dmg
    ;;

  xLinux|xSunOS)
    mkdir $build-$1
    cd $build-$1
 
    mkdir copasi
    mkdir copasi/bin
    mkdir copasi/share
    mkdir copasi/share/copasi
    mkdir copasi/share/copasi/doc
    mkdir copasi/share/copasi/doc/html
    mkdir copasi/share/copasi/doc/html/figures
    mkdir copasi/share/copasi/examples
    chmod -R 755 copasi

    cp ../README.$1 copasi/README
    chmod 644 copasi/README

    cp ../copasi/CopasiUI/CopasiUI  copasi/bin
    chmod 755 copasi/bin/CopasiUI

    cp ../TestSuite/distribution/* copasi/share/copasi/examples
    chmod 644 copasi/share/copasi/examples/*

    cp ../copasi/wizard/help_html/*.html copasi/share/copasi/doc/html
    chmod 644 copasi/share/copasi/doc/html/*.html

    cp ../copasi/wizard/help_html/figures/*.jpg \
       copasi/share/copasi/doc/html/figures
    chmod 644 copasi/share/copasi/doc/html/figures/*.jpg

    tar -czf ../Copasi-$build-$1.tar.gz copasi
  
    cd ..
    rm -rf $build-$1
    ;;
  esac

  scp Copasi-$build-$1.* \
      calvin.bioinformatics.vt.edu:/usr/local/apache/htdocs/calvin/copasi/alpha-test/$1
  rm Copasi-$build-$1.*
else
  echo usage: mkbuild.sh BUILD_OS
fi
popd

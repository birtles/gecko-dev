#!/bin/bash
set -x -e -v

pushd "${GECKO_PATH}"
./mach artifact toolchain -v $MOZ_TOOLCHAINS
mv wrench-deps/{vendor,.cargo,cargo-apk} gfx/wr
popd

pushd "${GECKO_PATH}/gfx/wr/wrench"
# The following maven links are equivalent to GRADLE_MAVEN_REPOSITORIES, try
# and keep in sync
cat > build.gradle.inc <<END
buildscript {
    repositories {
        maven{ url uri('file:${GECKO_PATH}/android-gradle-dependencies/google') }
        maven{ url uri('file:${GECKO_PATH}/android-gradle-dependencies/jcenter') }
    }
    dependencies {
        classpath 'com.android.tools.build:gradle:3.1.4'
    }
}
allprojects {
    repositories {
        maven{ url uri('file:${GECKO_PATH}/android-gradle-dependencies/google') }
        maven{ url uri('file:${GECKO_PATH}/android-gradle-dependencies/jcenter') }
    }
}
END
# These things come from the toolchain dependencies of the job that invokes
# this script (webrender-wrench-android-build).
export PATH="${PATH}:${GECKO_PATH}/rustc/bin"
export ANDROID_HOME="${GECKO_PATH}/android-sdk-linux"
export NDK_HOME="${GECKO_PATH}/android-ndk"
export CARGO_APK_GRADLE_COMMAND="${GECKO_PATH}/android-gradle-dependencies/gradle-dist/bin/gradle"
export CARGO_APK_BUILD_GRADLE_INC="${PWD}/build.gradle.inc"
../cargo-apk/bin/cargo-apk build --frozen --verbose
popd

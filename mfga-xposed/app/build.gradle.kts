plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
}

android {
    namespace = "com.mfga.xposed"
    compileSdk = 36

    defaultConfig {
        applicationId = "com.mfga.xposed"
        minSdk = 26          // 与 README 的“Android 8+ 适配”保持一致
        targetSdk = 36
        versionCode = 13
        versionName = "1.3"
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            // Xposed 模块需要可安装的签名 APK；
            // 不配置签名的话 assembleRelease 产出的是无法安装的未签名包
            signingConfig = signingConfigs.getByName("debug")
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }
    kotlinOptions {
        jvmTarget = "17"
    }

    sourceSets["main"].resources.srcDirs("src/main/resources")
}

dependencies {
    compileOnly("de.robv.android.xposed:api:82")
    compileOnly("io.github.libxposed:api:102.0.0")
}

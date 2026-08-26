package com.mfga.xposed.modern

import android.graphics.Typeface
import android.util.Log
import com.mfga.xposed.FontForceCore
import io.github.libxposed.api.XposedModule
import io.github.libxposed.api.XposedModuleInterface.PackageLoadedParam
import io.github.libxposed.api.XposedModuleInterface.PackageReadyParam

private const val TAG = "MFGA"

class ModernEntry : XposedModule() {

    override fun onPackageLoaded(param: PackageLoadedParam) {
        super.onPackageLoaded(param)
        log(Log.INFO, TAG, "MFGA v1.3 (modern) attach: " + param.packageName)
    }

    override fun onPackageReady(param: PackageReadyParam) {
        super.onPackageReady(param)

        val cl = param.classLoader

        // 单字体文件路径：createFromAsset / createFromFile 内部走 Typeface.Builder#build()
        runCatching {
            val builderClass = Class.forName("android.graphics.Typeface\$Builder", false, cl)
            hookAndReplace(builderClass.getDeclaredMethod("build"))
        }.onFailure { log(Log.WARN, TAG, "hook Typeface.Builder#build failed: $it") }

        // 多字重 font-family 路径（比如 res/font/inter.xml 这种声明了 regular/medium
        runCatching {
            val fallbackBuilderClass =
                Class.forName("android.graphics.Typeface\$CustomFallbackBuilder", false, cl)
            hookAndReplace(fallbackBuilderClass.getDeclaredMethod("build"))
        }.onFailure { log(Log.WARN, TAG, "hook Typeface.CustomFallbackBuilder#build failed: $it") }

        // 兜底静态工厂方法
        hookStaticFactory(cl, "createFromAsset")
        hookStaticFactory(cl, "createFromFile")
    }

    private fun hookStaticFactory(cl: ClassLoader, methodName: String) {
        runCatching {
            val typefaceClass = Class.forName("android.graphics.Typeface", false, cl)
            for (m in typefaceClass.declaredMethods) {
                if (m.name != methodName) continue
                // 只 hook 公开的静态工厂，避免动到内部实现细节（private 方法
                // 不经过 app 直接调用，hook 只会白白增加开销）
                if (!java.lang.reflect.Modifier.isPublic(m.modifiers)) continue
                if (!java.lang.reflect.Modifier.isStatic(m.modifiers)) continue
                hookAndReplace(m)
            }
        }.onFailure { log(Log.WARN, TAG, "hook Typeface.$methodName failed: $it") }
    }

    /** 统一的 hook 逻辑：deoptimize 绕过内联 + 把结果换成系统字体（保留原本 style/weight）。 */
    private fun hookAndReplace(m: java.lang.reflect.Executable) {
        deoptimize(m)
        hook(m).intercept { chain ->
            if (FontForceCore.isReplacing()) {
                return@intercept chain.proceed()
            }
            val original = chain.proceed() as? Typeface
            FontForceCore.systemReplacementFor(original)
        }
    }
}

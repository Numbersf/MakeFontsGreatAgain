import { exec, spawn, toast } from "./assets/kernelsu.js";

const LOCALES_BASE = "./strings/locales/";
const FALLBACK_LOCALE = "zh_CN";

// 与 lang.sh 相似的 locale 解析逻辑
async function resolveLocaleKey() {
    let raw = "";

    try {
        const r = await exec("getprop persist.sys.locale 2>/dev/null");
        raw = (typeof r === "string" ? r : (r.stdout ?? r.output ?? "")).trim();
        raw = raw.split(",")[0].replace(/-/g, "_");
        if (raw === "null" || raw.includes("Failed")) raw = "";
    } catch (_) {}

    if (!raw) {
        try {
            const r = await exec("settings get system system_locales 2>/dev/null");
            raw = (typeof r === "string" ? r : (r.stdout ?? r.output ?? "")).trim();
            raw = raw.split(",")[0].replace(/-/g, "_");
            if (raw === "null" || raw.includes("Failed")) raw = "";
        } catch (_) {}
    }

    if (!raw) return [FALLBACK_LOCALE];

    // 标准化：language 小写，Script/Region 按 lang.sh 规则处理
    const parts = raw.split("_");
    const langCode = parts[0].toLowerCase();
    let regionCode = "";
    if (parts.length >= 2) {
        const p1 = parts[1];
        // Script 段为 4 个字母（如 Hans），Region 段为 2 个字母（如 CN）
        regionCode = p1.length === 4
            ? (parts[2] ? parts[2].toUpperCase() : "")
            : p1.toUpperCase();
    }

    // 优先级：lang_REGION → lang（前缀匹配由加载循环处理）→ fallback
    const candidates = [];
    if (regionCode) candidates.push(`${langCode}_${regionCode}`);
    candidates.push(langCode);
    candidates.push(FALLBACK_LOCALE);
    return [...new Set(candidates)];
}

// 按候选列表顺序 fetch JSON，返回第一个成功的
// 对于纯 langCode（如 "zh"），额外尝试前缀匹配
async function loadTranslations(candidates) {
    const KNOWN_LOCALES = ["zh_CN", "ja_JP", "en_US", "ru_RU"];

    for (const key of candidates) {
        try {
            const res = await fetch(`${LOCALES_BASE}${key}.json`);
            if (res.ok) return await res.json();
        } catch (_) {}

        if (!key.includes("_")) {
            const prefix  = key.toLowerCase() + "_";
            const matched = KNOWN_LOCALES.find(k => k.toLowerCase().startsWith(prefix));
            if (matched) {
                try {
                    const res = await fetch(`${LOCALES_BASE}${matched}.json`);
                    if (res.ok) return await res.json();
                } catch (_) {}
            }
        }
    }

    return {};
}

// 将翻译表应用到 DOM
function applyTranslations(T) {
    document.querySelectorAll("[data-i18n]").forEach(el => {
        const key = el.getAttribute("data-i18n");
        if (T[key] !== undefined) el.innerHTML = T[key];
    });
    document.querySelectorAll("[data-i18n-placeholder]").forEach(el => {
        const key = el.getAttribute("data-i18n-placeholder");
        if (T[key] !== undefined) el.placeholder = T[key];
    });
}


document.addEventListener("DOMContentLoaded", async () => {

// 检测 locale → 加载对应 JSON → 翻译 DOM
const candidates = await resolveLocaleKey();
const T = await loadTranslations(candidates);
applyTranslations(T);

// 取翻译字符串，缺失时返回 key 自身
const t = key => T[key] ?? key;

// ── 安全工具 ─────────────────────────────────────────────
// HTML 转义：所有不可信文本进入 terminal 前必须先过这里，防 XSS
function escHtml(s) {
    return String(s).replace(/[&<>"']/g, c => ({
        '&': '&amp;', '<': '&lt;', '>': '&gt;',
        '"': '&quot;', "'": '&#39;'
    })[c]);
}

// POSIX 单引号转义：所有用户输入拼进 shell 命令前必须先过这里，防命令注入
function shq(s) {
    return "'" + String(s).replace(/'/g, "'\\''") + "'";
}

const fontInput = document.getElementById('font-input');
const disableBtn = document.getElementById('disable-font');
const restoreBtn = document.getElementById('restore-font');
const recolorInput = document.getElementById('recolor-input');
const applyRecolorBtn = document.getElementById('apply-recolor');
const gmsSwitch = document.getElementById('gms-switch');
const terminal = document.querySelector('.output-terminal-content');
const cleanBtn = document.querySelector('.terminal-buttons .clean-terminal');
const copyBtn = document.querySelector('.terminal-buttons .copy-terminal');

// 默认转义；调用方需要注入受信任的 HTML 片段时传 raw=true。
// 多行文本自动转换为 <br>。
function log(msg, raw = false) {
    const parts = String(msg).split('\n');
    terminal.innerHTML += (raw ? parts.join('<br>') : parts.map(escHtml).join('<br>')) + "<br>";
    terminal.scrollTop = terminal.scrollHeight;
}

function warn(msg) {
    terminal.innerHTML += `<span style="color:#ef4444;font-weight:bold">${escHtml(msg)}</span><br>`;
    terminal.scrollTop = terminal.scrollHeight;
}

async function ensureDir() {
    await exec('mkdir -p /data/adb/modules/MFGA/system/fonts');
}

// 逐行转义后再给 .disabled 行加高亮，杜绝把 ls 输出当 HTML 注入
function highlightDisabledFiles(lsOutput) {
    return String(lsOutput).split('\n').map(line => {
        const safe = escHtml(line);
        return /\.disabled\s*$/.test(line)
            ? `<span style="color:#eab308;font-weight:bold">${safe}</span>`
            : safe;
    }).join('<br>');
}

async function showCurrentFiles() {
    try {
        const ls = await exec('ls -l /data/adb/modules/MFGA/system/fonts 2>&1');

        let output = typeof ls === 'string'
            ? ls
            : (ls?.output ?? ls?.stdout ?? ls?.stderr ?? JSON.stringify(ls));
        output = output.trim();
        log(`${escHtml(t('LOG_FONT_DIR'))}<br>${highlightDisabledFiles(output)}`, true);
    } catch (e) {
        warn(`${t('LOG_DIR_FAIL')}${e}`);
    }
}

async function handleFont(action) {
    const name = fontInput.value.trim();
    if (!name) { toast(t('TOAST_INPUT_EMPTY')); return; }
    if (!/^.+\.(?:ttf|otf|ttc)(?:,\(.+\))?$/.test(name)) { toast(t('TOAST_INPUT_INVALID')); return; }

    if (/^\d{3}(\.\w+)?$/.test(name) && action === 'disable') {
        warn(t('LOG_WARN_MAIN_FONT').replace('{name}', name));
    }

    await ensureDir();
    const dir = '/data/adb/modules/MFGA/system/fonts';
    const filePath = `${dir}/${name}`;
    const backupDir = '/data/adb/modules/MFGA/uni_backup';
    await exec(`mkdir -p ${shq(backupDir)}`);

    const isDisable = action === 'disable';

    async function fileExists(path) {
        const result = await exec(`[ -f ${shq(path)} ] && echo yes || echo no`);
        let output = typeof result === 'string' ? result
            : (result?.stdout ?? result?.output ?? '');
        return output.trim() === 'yes';
    }

    const match = name.match(/^([\w.-]+)(?:,\(([^)]+)\))?$/);
    const filename = match[1];
    const ranges = match[2] ?? '';
    // 防路径穿越：文件名禁止 .. 或 /，否则 mv/cp 可能动到模块目录之外
    if (filename.includes('..') || filename.includes('/')) {
        toast(t('TOAST_INPUT_INVALID'));
        return;
    }
    const disabledPath = `${filePath}.disabled`;
    const backupPath = `${backupDir}/${filename}`;

    if (isDisable) {
        // 屏蔽逻辑
        const disabledExists = await fileExists(disabledPath);
        const backupExists = await fileExists(backupPath);
        if (disabledExists || backupExists) {
            warn(`字体 ${filename} ${t('LOG_REPEAT_BLOCK')}`);
            return;
        }
        // 检查是否有 Unicode 区间
        if (ranges) {
            // 有 Unicode 区间 → 调用 unicode_filter.sh
            const cmd = `sh /data/adb/modules/MFGA/unicode_filter.sh ${shq(filename)} ${shq(ranges)}`;
            log(`${t('LOG_TRY_DISABLE')}${filename}${t('LOG_RANGE')}${ranges}`);
            try {
                const result = await exec(cmd);
                let output = typeof result === 'string' ? result
                    : (result.stdout ?? result.output ?? JSON.stringify(result));
                output = output.trim();
                const exitCode = (typeof result === 'object' && result !== null)
                    ? (result.exitCode ?? result.code ?? result.exit_code ?? null)
                    : null;
                const succeeded = exitCode !== null ? exitCode === 0 : !output.includes('[✗]');
                if (succeeded) {
                    if (output) log(output);
                    log(`${t('LOG_DISABLED_OK')}${filename}`);
                } else {
                    if (output) warn(output);
                    warn(`${t('LOG_DISABLED_FAIL')}${filename}`);
                }
            } catch (e) {
                warn(`${t('LOG_DISABLED_ERR')}${filename}${t('LOG_ERR_SUFFIX')}${e}`);
            }
        } else {
            // 无 Unicode 区间 → 直接 mv 改名为 .disabled
            if (!(await fileExists(filePath))) {
                warn(`${t('LOG_NOT_FOUND_TEXT')}${filename}`);
                return;
            }
            try {
                const result = await exec(`mv ${shq(filePath)} ${shq(disabledPath)}`);
                const exitCode = (typeof result === 'object' && result !== null)
                    ? (result.errno ?? result.exitCode ?? result.code ?? result.exit_code ?? null)
                    : null;
                const ok = exitCode === null || exitCode === 0;
                if (ok) log(`[✓] ${t('LOG_DISABLED_TEXT')}${filename}`);
                else warn(`${t('LOG_DISABLED_FAIL')}${filename}`);
            } catch (e) {
                warn(`${t('LOG_DISABLED_FAIL')}${filename}${t('LOG_ERR_SUFFIX')}${e}`);
            }
            await showCurrentFiles();
        }

    } else {
        // 恢复逻辑
        const disabledExists = await fileExists(disabledPath);
        if (disabledExists) {
            // 重命名 .disabled 恢复
            try {
                const result = await exec(`mv ${shq(disabledPath)} ${shq(filePath)}`);
                const exitCode = (typeof result === 'object' && result !== null)
                    ? (result.errno ?? result.exitCode ?? result.code ?? result.exit_code ?? null)
                    : null;
                if (exitCode === null || exitCode === 0) {
                    log(`[✓] ${t('LOG_RESTORED')}${name}`);
                } else {
                    warn(`${t('LOG_RESTORE_FAIL')}${name}`);
                }
            } catch (e) {
                warn(`${t('LOG_RESTORE_FAIL')}${name}${t('LOG_ERR_SUFFIX')}${e}`);
            }
            await showCurrentFiles();
            return;
        }

        const backupExists = await fileExists(backupPath);
        if (!backupExists) {
            warn(`${t('LOG_NOT_FOUND_RESTORE')}${name}`);
            return;
        }
        if (!/.*\.(ttf|otf|ttc)$/i.test(name)) {
            warn(t('LOG_RANGE_RESTORE_HINT'));
            return;
        }
        // 从 uni_backup 复制恢复
        try {
            const result = await exec(`cp ${shq(backupPath)} ${shq(filePath)}`);
            const exitCode = (typeof result === 'object' && result !== null)
                ? (result.errno ?? result.exitCode ?? result.code ?? result.exit_code ?? null)
                : null;
            if (exitCode === null || exitCode === 0) {
                log(`[✓] ${t('LOG_RESTORED')}${filename}`);
            } else {
                warn(`${t('LOG_RESTORE_FAIL')}${name}`);
            }
        } catch (e) {
            warn(`${t('LOG_RESTORE_FAIL')}${name}${t('LOG_ERR_SUFFIX')}${e}`);
        }
        // 清掉备份
        const rm = await exec(`rm -f ${shq(backupPath)}`);
        if ((typeof rm === 'object' && rm !== null && (rm.errno ?? rm.exitCode ?? rm.code ?? rm.exit_code) !== 0)) {
            warn(`${t('LOG_BACKUP_MISSING')}`);
        }
    }
}

async function handleRecolor() {
    const raw = recolorInput.value.trim();
    if (!raw) { toast(t('TOAST_INPUT_EMPTY')); return; }
    if (!/^[uU]\+[0-9A-Fa-f]{1,6};#?[0-9A-Fa-f]{6}$/.test(raw)) {
        toast(t('TOAST_RECOLOR_INVALID'));
        return;
    }

    const descEl = document.getElementById('recolor-desc-text');
    applyRecolorBtn.classList.add('toggle-muted');
    log(`${t('LOG_TRY_RECOLOR')}${raw}`);

    try {
        const cmd = `sh /data/adb/modules/MFGA/recolor_glyph.sh ${shq(raw)}`;
        const result = await exec(cmd);
        let output = typeof result === 'string' ? result
            : (result.stdout ?? result.output ?? '');
        let errOutput = typeof result === 'object' ? (result.stderr ?? '') : '';
        const exitCode = (typeof result === 'object' && result !== null)
            ? (result.errno ?? result.exitCode ?? result.code ?? null)
            : null;

        output = (output || '').trim();
        errOutput = (errOutput || '').trim();

        // 逐行分析二进制输出，区分 APPLY / REVERT / REPLACE
        let hasRevert = false;
        let hasApply = false;
        let hasReplace = false;
        output.split('\n').forEach(line => {
            if (/\[REVERT\]/.test(line)) hasRevert = true;
            if (/\[APPLY\]/.test(line))  hasApply = true;
            if (/\[REPLACE\]/.test(line)) hasReplace = true;
            if (line) log(line);
        });

        const succeeded = exitCode !== null ? exitCode === 0 : !errOutput;
        if (succeeded) {
            if (errOutput) log(errOutput);
            if (hasRevert) {
                log(t('LOG_RECOLOR_REVERTED'));
                toast(t('TOAST_RECOLOR_REVERTED'));
                // 描述文字短暂变色提示"已还原"
                if (descEl) {
                    descEl.setAttribute('data-i18n', 'RECOLOR_DESC_REVERTED');
                    descEl.textContent = t('RECOLOR_DESC_REVERTED');
                    descEl.classList.add('is-revert');
                    setTimeout(() => {
                        descEl.setAttribute('data-i18n', 'RECOLOR_DESC');
                        descEl.textContent = t('RECOLOR_DESC');
                        descEl.classList.remove('is-revert');
                    }, 3000);
                }
            } else if (hasReplace) {
                log(t('LOG_RECOLOR_REPLACED'));
                toast(t('TOAST_RECOLOR_APPLIED'));
            } else {
                log(t('LOG_RECOLOR_DONE'));
                toast(t('TOAST_RECOLOR_APPLIED'));
            }
        } else {
            if (errOutput) warn(errOutput);
            warn(t('LOG_RECOLOR_FAIL'));
        }
    } catch (e) {
        warn(`${t('LOG_RECOLOR_ERR')}${e}`);
    } finally {
        applyRecolorBtn.classList.remove('toggle-muted');
    }
}

async function handleGms() {
    if (!gmsSwitch) { warn(t('LOG_NO_GMS_SWITCH')); return; }
    if (gmsSwitch.disabled) return;
    gmsSwitch.disabled = true;
    gmsSwitch.checked = true;

    const runScript = (cmd, args = [], envVars = {}) => new Promise((resolve, reject) => {
        const child = spawn(cmd, args, { env: envVars });
        const handleOutput = (stream, logger) => {
            stream.on('data', chunk => {
                const text = chunk.toString().trim();
                if (text) logger(text.replace(/\n/g, '<br>'));
            });
        };
        handleOutput(child.stdout, log);
        handleOutput(child.stderr, warn);
        child.on('exit',  code => resolve(code));
        child.on('error', err  => reject(err));
    });

    try {
        // 'web' 作为位置参数传入，而不是依赖 spawn 的 env（部分 WebUI 不支持 env）
        const code = await runScript('sh', ['/data/adb/modules/MFGA/action.sh', 'web']);
        log(`${t('LOG_GMS_DONE')}${code}`);
    } catch (err) {
        warn(`${t('LOG_GMS_ERR')}${err}`);
    } finally {
        gmsSwitch.checked  = false;
        gmsSwitch.disabled = false;
    }
}

// GMSF 开关
if (gmsSwitch) {
    gmsSwitch.addEventListener('change', () => {
        if (gmsSwitch.checked) handleGms();
    });
} else {
    console.warn("gmsSwitch 未找到，监听器未绑定");
}

// ── 内置字体权限应用列表 ─────────────────────────────────
const fontPermInput = document.getElementById('font-perm-input');
const fontPermAddBtn = document.getElementById('font-perm-add');
const fontPermRunBtn = document.getElementById('font-perm-run');
const fontPermList = document.getElementById('font-perm-list');

const FONT_PERM_CONFIG = '/data/adb/modules/MFGA/config/font_permission_apps';
const FONT_PERM_DEFAULTS = ['com.dragon.read', 'com.qidian.QDReader'];

async function loadFontPermApps() {
    const r = await exec(`cat ${shq(FONT_PERM_CONFIG)} 2>/dev/null || true`);
    const out = (typeof r === 'string' ? r : (r.stdout ?? '') || '').trim();
    const apps = out.split('\n').map(s => s.trim()).filter(s => s && !s.startsWith('#'));
    return apps.length ? apps : FONT_PERM_DEFAULTS;
}

function renderFontPermList(apps) {
    if (!fontPermList) return;
    fontPermList.innerHTML = apps.length
        ? apps.map(a => `<span class="chip">${escHtml(a)}</span>`).join(' ')
        : `<span class="font-perm-empty">${escHtml(t('FONT_PERM_EMPTY'))}</span>`;
}

async function refreshFontPermList() {
    try {
        renderFontPermList(await loadFontPermApps());
    } catch (e) {
        warn(`${t('LOG_FONT_PERM_FAIL')}${e}`);
    }
}

async function addFontPermApp() {
    const pkg = fontPermInput.value.trim();
    if (!pkg) { toast(t('TOAST_INPUT_EMPTY')); return; }
    if (!/^[a-zA-Z0-9._]+$/.test(pkg)) { toast(t('TOAST_FONT_PERM_INVALID')); return; }
    const apps = await loadFontPermApps();
    if (apps.includes(pkg)) { toast(t('TOAST_FONT_PERM_DUP')); return; }
    await exec(`mkdir -p /data/adb/modules/MFGA/config && echo ${shq(pkg)} >> ${shq(FONT_PERM_CONFIG)}`);
    fontPermInput.value = '';
    await refreshFontPermList();
    toast(t('TOAST_FONT_PERM_ADDED'));
}

async function runFontPerm() {
    try {
        const apps = await loadFontPermApps();
        for (const app of apps) {
            await exec(`find /data/user/*/${app} -type f \\( -iname "*.ttf" -o -iname "*.otf" -o -iname "*.ttc" \\) -exec chmod 000 {} \\; 2>/dev/null || true`);
        }
        log(`${t('LOG_FONT_PERM_DONE')}${apps.join(', ')}`);
        toast(t('TOAST_FONT_PERM_RUN'));
    } catch (e) {
        warn(`${t('LOG_FONT_PERM_ERR')}${e}`);
    }
}

if (fontPermAddBtn) fontPermAddBtn.addEventListener('click', addFontPermApp);
if (fontPermRunBtn) fontPermRunBtn.addEventListener('click', runFontPerm);
if (fontPermList) refreshFontPermList();

// 屏蔽和恢复按钮
if (disableBtn) disableBtn.addEventListener('click', () => handleFont('disable'));
if (restoreBtn) restoreBtn.addEventListener('click', () => handleFont('restore'));

// 字符颜色修改按钮
if (applyRecolorBtn) applyRecolorBtn.addEventListener('click', () => handleRecolor());

// 清理和复制按钮
if (cleanBtn) {
    cleanBtn.addEventListener('click', () => {
        terminal.innerHTML = '';
        toast(t('TOAST_CLEANED'));
    });
}
if (copyBtn) {
    copyBtn.addEventListener('click', () => {
        navigator.clipboard.writeText(terminal.innerText)
            .then(()  => toast(t('TOAST_COPIED')))
            .catch(()  => toast(t('TOAST_COPY_FAIL')));
    });
}

// 双指缩放终端
let initial = null;
let fontSize = 14;

terminal.addEventListener('touchstart', (e) => {
    if (e.touches.length === 2) {
        initial = Math.hypot(
            e.touches[0].clientX - e.touches[1].clientX,
            e.touches[0].clientY - e.touches[1].clientY
        );
        e.stopPropagation();
    }
});

terminal.addEventListener('touchmove', (e) => {
    if (initial && e.touches.length === 2) {
        const current = Math.hypot(
            e.touches[0].clientX - e.touches[1].clientX,
            e.touches[0].clientY - e.touches[1].clientY
        );
        const delta = current - initial;
        if (Math.abs(delta) > 8) {
            fontSize = Math.max(10, Math.min(24, fontSize + (delta > 0 ? 1 : -1)));
            terminal.style.fontSize = fontSize + 'px';
            initial = current;
        }
        e.stopPropagation();
        e.preventDefault();
    }
});

document.addEventListener('touchmove', (e) => {
    if (e.touches.length > 1 && !terminal.contains(e.target)) e.preventDefault();
}, { passive: false });

// 帮助面板
const helpOverlay = document.getElementById('help-overlay');
const helpBtn     = document.getElementById('help-btn');
const helpClose   = document.getElementById('help-close');

helpBtn.addEventListener('click', () => helpOverlay.classList.add('open'));

function closeHelp() { helpOverlay.classList.remove('open'); }
helpClose.addEventListener('click', closeHelp);
helpOverlay.addEventListener('click', (e) => { if (e.target === helpOverlay) closeHelp(); });

// 可复制的代码块
document.querySelectorAll('.help-code[data-copy]').forEach(el => {
    el.addEventListener('click', () => {
        const text = el.getAttribute('data-copy');
        navigator.clipboard.writeText(text)
            .then(() => toast(t('TOAST_SAMPLE_COPIED')))
            .catch(() => {
                const ta = document.createElement('textarea');
                ta.value = text;
                ta.style.position = 'fixed';
                ta.style.opacity = '0';
                document.body.appendChild(ta);
                ta.select();
                document.execCommand('copy');
                document.body.removeChild(ta);
                toast(t('TOAST_SAMPLE_COPIED'));
            });
    });
});

});
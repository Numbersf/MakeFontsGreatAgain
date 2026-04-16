import { exec, spawn, toast } from "./assets/kernelsu.js";

document.addEventListener("DOMContentLoaded", () => {

const fontInput = document.getElementById('font-input');
const disableBtn = document.getElementById('disable-font');
const restoreBtn = document.getElementById('restore-font');
const gmsSwitch = document.getElementById('gms-switch');
const terminal = document.querySelector('.output-terminal-content');
const clearBtn = document.querySelector('.terminal-buttons .clear-terminal');
const copyBtn = document.querySelector('.terminal-buttons .copy-terminal');

console.log("gmsSwitch =", gmsSwitch);

function log(msg) {
    terminal.innerHTML += msg + "<br>";
    terminal.scrollTop = terminal.scrollHeight;
}

function warn(msg) {
    terminal.innerHTML += `<span style="color:#ef4444;font-weight:bold">${msg}</span><br>`;
    terminal.scrollTop = terminal.scrollHeight;
}

async function ensureDir() {
    await exec('mkdir -p /data/adb/modules/MFGA/system/fonts');
}

function highlightDisabledFiles(lsOutput) {
    return lsOutput.replace(/(.*\.disabled)/g, '<span style="color:#eab308;font-weight:bold">$1</span>');
}

async function showCurrentFiles() {
    try {
        const ls = await exec('ls -l /data/adb/modules/MFGA/system/fonts 2>&1');

        let output = typeof ls === 'string'
            ? ls
            : (ls?.output ?? ls?.stdout ?? ls?.stderr ?? JSON.stringify(ls));
        output = output.trim();
        log(`[+] 当前字体目录:<br>${highlightDisabledFiles(output).replace(/\n/g, '<br>')}`);
    } catch (e) {
        warn(`[!] 读取目录失败: ${e}`);
    }
}

async function handleFont(action) {
    const name = fontInput.value.trim();
    if (!name) { toast('请输入字体文件喵'); return; }
    if (!/^.+\.(?:ttf|otf|ttc)(?:,\(.+\))?$/.test(name)) { toast('填写信息不合法'); return; }

    if (/^\d{3}(\.\w+)?$/.test(name) && action === 'disable') {
        warn(`[!] 检测到你选择了疑似主字体的文件 (${name})，如果确实是主字体，请立即恢复！`);
    }

    await ensureDir();
    const dir = '/data/adb/modules/MFGA/system/fonts';
    const filePath = `${dir}/${name}`;
    const backupDir = '/data/adb/modules/MFGA/uni_backup';
    await exec(`mkdir -p "${backupDir}"`);

    const isDisable = action === 'disable';

    async function fileExists(path) {
        const result = await exec(`[ -f "${path}" ] && echo yes || echo no`);
        let output = '';
        if (typeof result === 'string') output = result;
        else if (result && typeof result === 'object') output = result.stdout ?? result.output ?? '';
        return output.trim() === 'yes';
    }

    const match = name.match(/^([\w.-]+)(?:,\(([^)]+)\))?$/);
    const filename = match[1];
    const ranges = match[2] ?? '';
    const disabledPath = `${filePath}.disabled`;
    const backupPath = `${backupDir}/${filename}`;

    if (isDisable) {
        // 屏蔽逻辑
        const disabledExists = await fileExists(disabledPath);
        const backupExists = await fileExists(backupPath);
        if (disabledExists || backupExists) {
            warn(`字体 ${filename} 尚未恢复，禁止重复屏蔽！`);
            return;
        }
        // 检查是否有 Unicode 区间
        if (ranges) {
            // 有 Unicode 区间 → 调用 unicode_filter.sh
            const cmd = `sh /data/adb/modules/MFGA/unicode_filter.sh "${filename}" "${ranges}"`;
            log(`[+] 尝试屏蔽: ${filename}, 区间: ${ranges}`);
            try {
                const result = await exec(cmd);
                let output = typeof result === 'string' ? result : (result.stdout ?? result.output ?? JSON.stringify(result));
                output = output.trim();
                const exitCode = (typeof result === 'object' && result !== null)
                    ? (result.exitCode ?? result.code ?? result.exit_code ?? null)
                    : null;
                const succeeded = exitCode !== null ? exitCode === 0 : !output.includes('[✗]');
                const htmlOutput = output.replace(/\n/g, '<br>');
                if (succeeded) {
                    if (htmlOutput) log(htmlOutput);
                    log(`[✓] 屏蔽成功: ${filename}`);
                } else {
                    if (htmlOutput) warn(htmlOutput);
                    warn(`[✗] 屏蔽失败: ${filename}`);
                }
            } catch (e) {
                warn(`[✗] 屏蔽异常: ${filename}, 错误: ${e}`);
            }
        } else {
            // 无 Unicode → 直接 mv 改名为 .disabled
            try {
                const result = await exec(`[ -f "${filePath}" ] && mv "${filePath}" "${disabledPath}" && echo "已屏蔽: ${filename}" || echo "未找到: ${filename}"`);
                let output = typeof result === 'string' ? result : (result.stdout ?? result.output ?? '');
                output = output.trim();
                if (output.includes('已屏蔽')) log(`[✓] ${output}`);
                else if (output.includes('未找到')) warn(`[!] ${output}`);
            } catch (e) {
                warn(`[✗] 屏蔽失败: ${filename}, 错误: ${e}`);
            }
        }

    } else {
        // 恢复逻辑
        let restoreCmd = '';

        const disabledExists = await fileExists(disabledPath);
        if (disabledExists) {
            // 优先 .disabled
            restoreCmd = `mv "${disabledPath}" "${filePath}" && echo "已恢复: ${name}"`;
        } else {
            const backupExists = await fileExists(backupPath);
            if (backupExists) {
                // 其次 uni_backup
                if (/.*\.(ttf|otf|ttc)$/i.test(name)) {
                    restoreCmd = `cp "${backupPath}" "${filePath}" && echo "已恢复: ${filename}"`;
                } else {
                    warn('[!] 有区间的字体恢复只需输入字体文件名');
                    return;
                }
            } else {
                warn(`[!] 未找到可恢复的字体: ${name}`);
                return;
            }
        }

        try {
            const result = await exec(restoreCmd);
            let output = typeof result === 'string' ? result : (result.stdout ?? result.output ?? '');
            output = output.trim();
            if (output) log(`[✓] ${output}`);
            // 删除备份文件
            await exec(`[ -f "${backupPath}" ] && rm "${backupPath}" || echo "备份文件不存在"`);
        } catch (e) {
            warn(`[✗] 恢复失败: ${name}, 错误: ${e}`);
        }
    }

    // 最后显示当前字体目录
    await showCurrentFiles();
}

async function handleGms() {
    if (!gmsSwitch) {
        warn("[✗] 未找到 gms-switch 元素");
        return;
    }

    if (gmsSwitch.disabled) return;
    gmsSwitch.disabled = true;
    gmsSwitch.checked = true;

    const runScript = (cmd, args = [], envVars = {}) => {
        return new Promise((resolve, reject) => {
            const child = spawn(cmd, args, { env: envVars });

            // 统一处理 stdout / stderr 输出
            const handleOutput = (stream, logger) => {
                stream.on('data', chunk => {
                    const text = chunk.toString().trim();
                    if (text) logger(text.replace(/\n/g, '<br>'));
                });
            };
            handleOutput(child.stdout, log);
            handleOutput(child.stderr, warn);

            child.on('exit', code => resolve(code));
            child.on('error', err => reject(err));
        });
    };

    try {
        const code = await runScript('sh', ['/data/adb/modules/MFGA/action.sh'], { MODE: 'web' });
        log(`[✓] GMSF 清理屏蔽完成, 退出码: ${code}`);
    } catch (err) {
        warn(`[✗] GMSF 执行异常: ${err}`);
    } finally {
        // 无论成功或失败，都恢复按钮状态
        gmsSwitch.checked = false;
        gmsSwitch.disabled = false;
    }
}

// GMSF开关
if (gmsSwitch) {
    gmsSwitch.addEventListener('change', () => {
        if (gmsSwitch.checked) handleGms(); // 勾选立即触发
    });
} else {
    console.warn("gmsSwitch 未找到，监听器未绑定");
}

// 屏蔽和恢复按钮
if (disableBtn) disableBtn.addEventListener('click', () => handleFont('disable'));
if (restoreBtn) restoreBtn.addEventListener('click', () => handleFont('restore'));

// 清理和复制按钮
if (clearBtn) {
    clearBtn.addEventListener('click', () => {
        terminal.innerHTML = '';
        toast('清除完成喵');
    });
}
if (copyBtn) copyBtn.addEventListener('click', () => {
    navigator.clipboard.writeText(terminal.innerText)
        .then(() => toast('已复制输出内容喵'))
        .catch(() => toast('复制失败'));
});

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

document.addEventListener('touchmove', function (e) {
    if (e.touches.length > 1 && !terminal.contains(e.target)) e.preventDefault();
}, { passive: false });

// 帮助面板
const helpOverlay = document.getElementById('help-overlay');
const helpBtn     = document.getElementById('help-btn');
const helpClose   = document.getElementById('help-close');

helpBtn.addEventListener('click', () => {
    helpOverlay.classList.add('open');
});

function closeHelp() {
    helpOverlay.classList.remove('open');
}

helpClose.addEventListener('click', closeHelp);

// 点击遮罩关闭
helpOverlay.addEventListener('click', (e) => {
    if (e.target === helpOverlay) closeHelp();
});

// 可复制的代码块
document.querySelectorAll('.help-code[data-copy]').forEach(el => {
    el.addEventListener('click', () => {
        const text = el.getAttribute('data-copy');
        navigator.clipboard.writeText(text)
            .then(() => toast('已复制样本喵'))
            .catch(() => {
                // 降级方案
                const ta = document.createElement('textarea');
                ta.value = text;
                ta.style.position = 'fixed';
                ta.style.opacity = '0';
                document.body.appendChild(ta);
                ta.select();
                document.execCommand('copy');
                document.body.removeChild(ta);
                toast('已复制样本喵');
            });
    });
});

});
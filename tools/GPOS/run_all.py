# -*- coding: utf-8 -*-
import step1_diff_glyphs
import step2_merge_glyphs
import step3_build_gpos
import step4_validate
import config


def main():
    print("对比源字体与修改字体")
    step1_diff_glyphs.diff_all()

    print()
    print("搬运新增/改动字形")
    step2_merge_glyphs.merge_all()

    print()
    print("建立完整 GPOS 组合记号锚点")
    step3_build_gpos.build_all(input_dir=config.MERGED_DIR, output_dir=config.RESULT_DIR)

    print()
    print("最后校验")
    ok = step4_validate.validate_all()

    print()
    if ok:
        print(f"全部完成，结果: {config.RESULT_DIR}")
    else:
        print("校验未通过，请检查 [FAIL] 标记。")


if __name__ == "__main__":
    main()

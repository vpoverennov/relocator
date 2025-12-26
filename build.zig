const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{
        .whitelist = &.{
            .{
                .os_tag = .windows,
                .cpu_arch = .x86_64,
            },
        },
    });
    const optimize = b.standardOptimizeOption(.{
        .preferred_optimize_mode = .ReleaseSmall,
    });

    const root_module = b.createModule(.{
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    root_module.addCSourceFile(.{
        .file = b.path("relocator.c"),
        .flags = &.{
            "-Wall",
            "-Wextra",
            "-Wpedantic",
            "-Werror",
        },
    });
    root_module.addCMacro("UNICODE", "1");
    root_module.addCMacro("_UNICODE", "1");
    root_module.addWin32ResourceFile(.{
        .file = b.path("rl.rc"),
    });

    const exe = b.addExecutable(.{
        .name = "rl",
        .root_module = root_module,
    });
    exe.subsystem = .Windows;

    b.installArtifact(exe);
}

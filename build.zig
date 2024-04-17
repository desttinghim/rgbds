const std = @import("std");

pub fn build(b: *std.Build) !void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    // Run bison
    bison: {
        const update_parser = b.step("update-parser", "Runs system installed bison to regenerate src/asm/parser.c");

        const version = parseBisonVersion(b) catch break :bison;
        const flags_version = &[_][2]usize{
            .{ 3, 5 },
            .{ 3, 6 },
            .{ 3, 0 },
            .{ 3, 0 },
            .{ 3, 0 },
        };
        const flags = &[_][]const u8{
            "-Dapi.token.raw=true",
            "-Dparse.error=detailed",
            "-Dparse.error=verbose",
            "-Dparse.lac=full",
            "-Dlr.type=ielr",
        };

        const bison = b.addSystemCommand(&.{"bison"});
        for (flags, flags_version) |flag, v| {
            if (v[0] == version.major and v[1] == version.minor) {
                bison.addArg(flag);
            }
        }
        bison.addArg("-d");
        bison.addArg("-Wall");
        const parser_c = bison.addPrefixedOutputFileArg("--output=", "parser.c");
        const parser_h = bison.addPrefixedOutputFileArg("--header=", "parser.h");
        bison.addFileArg(.{ .path = "src/asm/parser.y" });

        const write_files = b.addNamedWriteFiles("bison");
        write_files.addCopyFileToSource(parser_c, "src/asm/parser.c");
        write_files.addCopyFileToSource(parser_h, "src/asm/parser.h");
        update_parser.dependOn(&write_files.step);
    }

    {
        const exe = b.addExecutable(.{
            .name = "rgbasm",
            .link_libc = true,
            .target = target,
            .optimize = optimize,
        });
        exe.addCSourceFiles(.{
            .files = files_rgbasm,
            .flags = &.{
                "-std=gnu11",
            },
        });
        exe.addIncludePath(.{ .path = "src" });
        exe.addIncludePath(.{ .path = "include" });
        exe.addIncludePath(.{ .path = "src/asm" });

        b.installArtifact(exe);

        const run_step = b.step("asm", "Run rgbasm");

        const run = b.addRunArtifact(exe);
        run_step.dependOn(&run.step);
    }

    {
        const exe = b.addExecutable(.{
            .name = "rgblink",
            .link_libc = true,
            .target = target,
            .optimize = optimize,
        });
        exe.addCSourceFiles(.{
            .files = files_rgblink,
            .flags = &.{
                "-std=gnu11",
            },
        });
        exe.addIncludePath(.{ .path = "src" });
        exe.addIncludePath(.{ .path = "include" });
        exe.addIncludePath(.{ .path = "src/link" });

        b.installArtifact(exe);

        const run_step = b.step("link", "Run rgblink");

        const run = b.addRunArtifact(exe);
        run_step.dependOn(&run.step);
    }

    {
        const exe = b.addExecutable(.{
            .name = "rgbfix",
            .link_libc = true,
            .target = target,
            .optimize = optimize,
        });
        exe.addCSourceFiles(.{
            .files = files_rgbfix,
            .flags = &.{
                "-std=gnu11",
            },
        });
        exe.addIncludePath(.{ .path = "src" });
        exe.addIncludePath(.{ .path = "include" });
        exe.addIncludePath(.{ .path = "src/fix" });

        b.installArtifact(exe);

        const run_step = b.step("fix", "Run rgbfix");

        const run = b.addRunArtifact(exe);
        run_step.dependOn(&run.step);
    }
}

fn parseBisonVersion(b: *std.Build) !std.SemanticVersion {
    // First line of bison --version:
    // bison (GNU Bison) 3.8.2
    var out_code: u8 = 0;
    const output = try b.runAllowFail(&.{ "bison", "--version" }, &out_code, .Ignore);
    const first_line = std.mem.sliceTo(output, '\n');
    var iter = std.mem.splitBackwardsScalar(u8, first_line, ' ');
    const version_string = iter.next().?;
    return std.SemanticVersion.parse(version_string);
}

const files_rgbasm = &[_][]const u8{
    "src/asm/charmap.c",
    "src/asm/fixpoint.c",
    "src/asm/format.c",
    "src/asm/fstack.c",
    "src/asm/lexer.c",
    "src/asm/macro.c",
    "src/asm/main.c",
    "src/asm/opt.c",
    "src/asm/output.c",
    "src/asm/parser.c",
    "src/asm/rpn.c",
    "src/asm/section.c",
    "src/asm/symbol.c",
    "src/asm/util.c",
    "src/asm/warning.c",
    "src/extern/getopt.c",
    "src/extern/utf8decoder.c",
    "src/error.c",
    "src/hashmap.c",
    "src/linkdefs.c",
    "src/opmath.c",
    "src/version.c",
};

const files_rgblink = &[_][]const u8{
    "src/link/assign.c",
    "src/link/main.c",
    "src/link/object.c",
    "src/link/output.c",
    "src/link/patch.c",
    "src/link/script.c",
    "src/link/sdas_obj.c",
    "src/link/section.c",
    "src/link/symbol.c",
    "src/extern/getopt.c",
    "src/extern/utf8decoder.c",
    "src/error.c",
    "src/hashmap.c",
    "src/linkdefs.c",
    "src/opmath.c",
    "src/version.c",
};

const files_rgbfix = &[_][]const u8{
    "src/fix/main.c",
    "src/extern/getopt.c",
    "src/error.c",
    "src/version.c",
};

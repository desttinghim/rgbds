const std = @import("std");

pub fn build(b: *std.Build) !void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    // Run bison
    bison: {
        const run_bison = b.step("run-bison", "Runs system installed bison to turn .y files into .cpp/.hpp");

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

        {
            const bison = b.addSystemCommand(&.{"bison"});
            for (flags, flags_version) |flag, v| {
                if (v[0] == version.major and v[1] == version.minor) {
                    bison.addArg(flag);
                }
            }
            bison.addArg("-d");
            bison.addArg("-Wall");
            const parser_c = bison.addPrefixedOutputFileArg("--output=", "parser.cpp");
            const parser_h = bison.addPrefixedOutputFileArg("--header=", "parser.hpp");
            bison.addFileArg(.{ .path = "src/asm/parser.y" });

            const write_files = b.addNamedWriteFiles("bison parser");
            write_files.addCopyFileToSource(parser_c, "src/asm/parser.cpp");
            write_files.addCopyFileToSource(parser_h, "src/asm/parser.hpp");
            run_bison.dependOn(&write_files.step);
        }
        {
            const bison = b.addSystemCommand(&.{"bison"});
            for (flags, flags_version) |flag, v| {
                if (v[0] == version.major and v[1] == version.minor) {
                    bison.addArg(flag);
                }
            }
            bison.addArg("-d");
            bison.addArg("-Wall");
            const parser_c = bison.addPrefixedOutputFileArg("--output=", "script.cpp");
            const parser_h = bison.addPrefixedOutputFileArg("--header=", "script.hpp");
            bison.addFileArg(.{ .path = "src/link/script.y" });

            const write_files = b.addNamedWriteFiles("bison script");
            write_files.addCopyFileToSource(parser_c, "src/link/script.cpp");
            write_files.addCopyFileToSource(parser_h, "src/link/script.hpp");
            run_bison.dependOn(&write_files.step);
        }
    }

    {
        const exe = b.addExecutable(.{
            .name = "rgbasm",
            .link_libc = true,
            .target = target,
            .optimize = optimize,
        });
        exe.linkLibCpp();
        exe.addCSourceFiles(.{
            .files = files_rgbasm,
            .flags = &.{
                "-std=c++2a",
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
        exe.linkLibCpp();
        exe.addCSourceFiles(.{
            .files = files_rgblink,
            .flags = &.{
                "-std=c++2a",
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
        exe.linkLibCpp();
        exe.addCSourceFiles(.{
            .files = files_rgbfix,
            .flags = &.{
                "-std=c++2a",
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

    {
        const libpng_dep = b.dependency("libpng", .{
            .target = target,
            .optimize = optimize,
        });

        const exe = b.addExecutable(.{
            .name = "rgbgfx",
            .link_libc = true,
            .target = target,
            .optimize = optimize,
        });
        exe.linkLibrary(libpng_dep.artifact("png"));
        exe.linkLibCpp();
        exe.addCSourceFiles(.{
            .files = files_rgbgfx,
            .flags = &.{
                "-std=c++2a",
            },
        });
        exe.addIncludePath(.{ .path = "src" });
        exe.addIncludePath(.{ .path = "include" });
        exe.addIncludePath(.{ .path = "src/gfx" });

        b.installArtifact(exe);

        const run_step = b.step("gfx", "Run rgbgfx");

        const run = b.addRunArtifact(exe);
        run_step.dependOn(&run.step);

        if (b.args) |args| {
            run.addArgs(args);
        }
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
    "src/asm/charmap.cpp",
    "src/asm/fixpoint.cpp",
    "src/asm/format.cpp",
    "src/asm/fstack.cpp",
    "src/asm/lexer.cpp",
    "src/asm/macro.cpp",
    "src/asm/main.cpp",
    "src/asm/opt.cpp",
    "src/asm/output.cpp",
    "src/asm/parser.cpp",
    "src/asm/rpn.cpp",
    "src/asm/section.cpp",
    "src/asm/symbol.cpp",
    "src/asm/warning.cpp",
    "src/extern/getopt.cpp",
    "src/extern/utf8decoder.cpp",
    "src/error.cpp",
    "src/hashmap.cpp",
    "src/linkdefs.cpp",
    "src/opmath.cpp",
    "src/util.cpp",
    "src/version.cpp",
};

const files_rgblink = &[_][]const u8{
    "src/link/assign.cpp",
    "src/link/main.cpp",
    "src/link/object.cpp",
    "src/link/output.cpp",
    "src/link/patch.cpp",
    "src/link/script.cpp",
    "src/link/sdas_obj.cpp",
    "src/link/section.cpp",
    "src/link/symbol.cpp",
    "src/extern/getopt.cpp",
    "src/extern/utf8decoder.cpp",
    "src/error.cpp",
    "src/hashmap.cpp",
    "src/linkdefs.cpp",
    "src/opmath.cpp",
    "src/util.cpp",
    "src/version.cpp",
};

const files_rgbfix = &[_][]const u8{
    "src/fix/main.cpp",
    "src/extern/getopt.cpp",
    "src/error.cpp",
    "src/version.cpp",
};

const files_rgbgfx = &[_][]const u8{
    "src/gfx/main.cpp",
    "src/gfx/pal_packing.cpp",
    "src/gfx/pal_sorting.cpp",
    "src/gfx/pal_spec.cpp",
    "src/gfx/process.cpp",
    "src/gfx/proto_palette.cpp",
    "src/gfx/reverse.cpp",
    "src/gfx/rgba.cpp",
    "src/extern/getopt.cpp",
    "src/error.cpp",
    "src/version.cpp",
};

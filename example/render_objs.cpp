#include <ctime>
#include <string>

#include <CLI/CLI.hpp>
#include <ghc/fs_std.hpp> // filesystem
#include <igl/Timer.h>
#include <igl/edges.h>
#include <igl/readOBJ.h>
#include <nlohmann/json.hpp>
// clang-format off
#include <spdlog/spdlog.h>
#include <spdlog/fmt/chrono.h>
// clang-format on
#include <tbb/parallel_for.h>
#include <tbb/parallel_sort.h>

#include <swr/render_mesh.hpp>

using namespace swr;

struct SimRenderArgs {
    fs::path obj_path;
    fs::path output_path = fs::path("obj_seq.mp4");
    fs::path render_settings_path =
        fs::path(__FILE__).parent_path() / "render_settings.json";
    spdlog::level::level_enum loglevel = spdlog::level::level_enum::info;
    int fps = -1;
};

SimRenderArgs parse_args(int argc, char* argv[])
{
    CLI::App app { "render simulation headless" };

    SimRenderArgs args;

    app.add_option(
           "obj_path,-i,--input", args.obj_path,
           "path to folder with a sequence of OBJs")
        ->required()
        ->check(CLI::ExistingDirectory);
    app.add_option("-o,--output", args.output_path, "path to output render");
    app.add_option(
        "-l,--log,--loglevel", args.loglevel,
        "set log level 0=trace, 1=debug, 2=info, 3=warn, 4=error, 5=critical, "
        "6=off");
    app.add_option("--fps", args.fps, "output video frames per second");
    app.add_option("--settings", args.render_settings_path, "render settings");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        exit(app.exit(e));
    }

    return args;
}

bool read_json(const std::string& filename, nlohmann::json& json)
{
    std::ifstream input(filename);
    json = nlohmann::json::parse(input, nullptr, false);
    return !json.is_discarded();
}

class MeshGenerator {
public:
    virtual ~MeshGenerator() {};
    virtual size_t num_meshes() = 0;
    virtual Eigen::MatrixXd vertices(size_t i) = 0;
    virtual Eigen::MatrixXi edges(size_t i) = 0;
    virtual Eigen::MatrixXi faces(size_t i) = 0;
    virtual Eigen::VectorXi colors(size_t i) = 0;
    virtual int fps() = 0;
};

class OBJSequence : public MeshGenerator {
public:
    OBJSequence(const fs::path& input)
    {
        std::vector<fs::path> objs;
        for (const auto& entry : fs::directory_iterator(input)) {
            if (entry.path().extension().string() == ".obj"
                || entry.path().extension().string() == ".OBJ") {
                objs.push_back(entry.path());
            }
        }
        tbb::parallel_sort(
            objs.begin(), objs.end(), [](const fs::path& a, const fs::path& b) {
                try {
                    return std::stoi(a.stem().string())
                        < std::stoi(b.stem().string());
                } catch (...) {
                    return a < b;
                }
            });
        for (const auto& obj : objs) {
            spdlog::info("Loading OBJ: \"{}\"", obj.string());
            vertices_sequence.emplace_back();
            edges_sequence.emplace_back();
            faces_sequence.emplace_back();
            igl::readOBJ(
                obj.string(), vertices_sequence.back(), faces_sequence.back());
            igl::edges(faces_sequence.back(), edges_sequence.back());
        }
    }

    virtual ~OBJSequence() override {};

    size_t num_meshes() override { return vertices_sequence.size(); }

    Eigen::MatrixXd vertices(size_t i) override
    {
        assert(i < num_meshes());
        return vertices_sequence[i];
    }

    Eigen::MatrixXi edges(size_t i) override
    {
        assert(i < num_meshes());
        return edges_sequence[i];
    }

    Eigen::MatrixXi faces(size_t i) override
    {
        assert(i < num_meshes());
        return faces_sequence[i];
    }

    Eigen::VectorXi colors(size_t i) override
    {
        assert(i < num_meshes());
        return Eigen::VectorXi::Constant(vertices_sequence[i].rows(), 2);
    }

    int fps() override { return 60; }

protected:
    std::vector<Eigen::MatrixXd> vertices_sequence;
    std::vector<Eigen::MatrixXi> edges_sequence;
    std::vector<Eigen::MatrixXi> faces_sequence;
};

int main(int argc, char* argv[])
{
    SimRenderArgs args = parse_args(argc, argv);

    spdlog::set_level(args.loglevel);

    ///////////////////////////////////////////////////////////////////////////
    // Create folder for PNG frames
    // Create the output directory if it does not exist
    if (args.output_path.has_parent_path()) {
        fs::create_directories(args.output_path.parent_path());
    }
    // std::string current_time_string =
    //     fmt::format("{:%F-%H-%M-%S}", fmt::localtime(std::time(0)));
    // fs::path frames_dir = args.output_path.parent_path()
    //     / fmt::format("frames-{}", current_time_string);
    fs::path frames_dir = args.output_path.parent_path() / "frames";
    fs::create_directories(frames_dir);
    ///////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////
    // Determine if the input is a simulation json or sequence of OBJs
    std::unique_ptr<MeshGenerator> mesh_generator;
    if (fs::is_directory(args.obj_path)) {
        mesh_generator = std::make_unique<OBJSequence>(args.obj_path);
    } else {
    }
    if (mesh_generator->num_meshes() == 0) {
        return 0;
    }
    ///////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////
    // Create a scene
    nlohmann::json render_args;
    if (!read_json(args.render_settings_path.string(), render_args)) {
        spdlog::error(
            "Invalid render settings JSON file ({})",
            args.render_settings_path.string());
        return 1;
    }
    Scene scene(render_args);
    scene.camera.align_camera_center(
        mesh_generator->vertices(0), mesh_generator->faces(0));
    ///////////////////////////////////////////////////////////////////////////

    tbb::parallel_for(size_t(0), mesh_generator->num_meshes(), [&](size_t i) {
        std::string frame_name =
            (frames_dir / fmt::format("frame{:06d}.png", i)).string();

        igl::Timer render_timer;
        render_timer.start();
        bool wrote_frame = render_mesh(
            scene, mesh_generator->vertices(i), mesh_generator->edges(i),
            mesh_generator->faces(i), mesh_generator->colors(i), frame_name);
        render_timer.stop();

        if (wrote_frame) {
            spdlog::info(
                "Rendered frame {:d} to '{}' in {:g} seconds", //
                i, frame_name, render_timer.getElapsedTime());
        } else {
            spdlog::error("Unable to render frame {:d} to '{}'", i, frame_name);
        }
    });

    ///////////////////////////////////////////////////////////////////////////
    // Combine PNG frames to video
    int fps = args.fps > 0 ? args.fps : render_args["fps"].get<int>();
    if (fps <= 0) {
        fps = mesh_generator->fps();
    }
    std::string ffmpeg_cmd = fmt::format(
        "ffmpeg -hide_banner -loglevel warning -y -r {:d} -i {}/frame%06d.png "
        "-vcodec libx264 -crf 0 {}",
        fps, frames_dir.string(), args.output_path.string());
    spdlog::info("Combining frames using '{}'", ffmpeg_cmd);
    std::system(ffmpeg_cmd.c_str());
    ///////////////////////////////////////////////////////////////////////////
}

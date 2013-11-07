exename = File.absolute_path("./rtsr.out")
fail unless File.exists?(exename)

Dir.chdir(ARGV[0])
patients=Dir.glob("patient*")
patients.each { |patient|
    puts "Processing #{patient}..."
        Dir.mkdir("#{patient}/volumes") unless Dir.exists?("#{patient}/volumes")
        exams=Dir.glob("#{patient}/p_*")
        exams.each { |exam|
            contours=Dir.glob("#{exam}/RTSTRUCT.*.dcm")
            contours.each { |cc|
                dirname=File.join(ARGV[0], patient,"volumes", File.basename(cc,".dcm").sub(/RTSTRUCT./, ''))
                Dir.mkdir(dirname) unless Dir.exists?(dirname)

                dcm_file=File.absolute_path(cc)
                system  "#{exename} #{dcm_file} #{dirname}"
            }
        }
}

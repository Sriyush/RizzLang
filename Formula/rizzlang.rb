class Rizzlang < Formula
  desc "Rizz++ - small GenZ-style toy language"
  homepage "https://github.com/Sriyush/RizzLang"
  url "https://github.com/Sriyush/RizzLang/releases/download/v1.0.12/rizz"
  sha256 "<SHA256>"
  version "VERSION"

  def install
    bin.install "rizz"
  end

  test do
    system "#{bin}/rizz", "--version"
  end
end

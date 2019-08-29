package io.github.hcoona.ini;

import io.github.hcoona.ini.IniParser.Event;

import java.util.Map;
import java.util.Objects;
import java.util.Set;
import java.util.function.Function;
import java.util.function.Supplier;

public class IniReader<TSection, TOptionKey, TOptionValue>
    implements AutoCloseable {
  private final IniParser iniParser;
  private final Supplier<Map<TSection, IniSectionObject<TSection, TOptionKey, TOptionValue>>>
      sectionMapSupplier;
  private final Supplier<Set<IniOptionObject<TOptionKey, TOptionValue>>> optionSetSupplier;
  private final Function<String, TSection> sectionFunction;
  private final Function<String, TOptionKey> optionKeyFunction;
  private final Function<String, TOptionValue> optionValueFunction;

  public IniReader(IniParser iniParser,
      Supplier<Map<TSection, IniSectionObject<TSection, TOptionKey, TOptionValue>>> sectionMapSupplier,
      Supplier<Set<IniOptionObject<TOptionKey, TOptionValue>>> optionSetSupplier,
      Function<String, TSection> sectionFunction,
      Function<String, TOptionKey> optionKeyFunction,
      Function<String, TOptionValue> optionValueFunction) {
    this.iniParser = iniParser;
    this.sectionMapSupplier = sectionMapSupplier;
    this.optionSetSupplier = optionSetSupplier;
    this.sectionFunction = sectionFunction;
    this.optionKeyFunction = optionKeyFunction;
    this.optionValueFunction = optionValueFunction;
  }

  @Override
  public void close() throws Exception {
    iniParser.close();
  }

  public IniFileObject<TSection, TOptionKey, TOptionValue> read() {
    final IniFileObject<TSection, TOptionKey, TOptionValue> fileObject =
        new IniFileObject<>(sectionMapSupplier.get());

    IniSectionObject<TSection, TOptionKey, TOptionValue> currentSection = null;
    String currentKey = null;
    String currentValue = null;

    while (iniParser.hasNext()) {
      final Event event = iniParser.next();
      switch (event) {
        case START_SECTION:
          break;
        case VISIT_SECTION_HEADER:
          currentSection = new IniSectionObject<>(
              sectionFunction.apply(iniParser.getSectionHeader()),
              optionSetSupplier.get());
          break;
        case END_SECTION:
          Objects.requireNonNull(currentSection);
          fileObject.put(currentSection.getKey(), currentSection);
          currentSection = null;
          break;
        case START_OPTION:
          break;
        case VISIT_KEY:
          currentKey = iniParser.getOptionKey();
          break;
        case VISIT_VALUE:
          currentValue = iniParser.getOptionValue();
          break;
        case END_OPTION:
          Objects.requireNonNull(currentKey);
          Objects.requireNonNull(currentValue);
          Objects.requireNonNull(currentSection).add(
              new IniOptionObject<>(
                  optionKeyFunction.apply(currentKey),
                  optionValueFunction.apply(currentValue)));
          currentKey = null;
          currentValue = null;
          break;
        case VISIT_COMMENT:
          break;
      }
    }

    return fileObject;
  }
}
